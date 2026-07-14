#include "scene.h"

#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <utility>

#include "../profiling/profiler.h"
#include "component_registry.h"
#include "i_savable.h"

Entity Scene::createEntity() { return Entity(m_nextEntityId++); }

void Scene::destroyEntity(Entity e)
{
    for (auto& [type, pool] : m_pools)
    {
        pool->remove(e);
    }
}

void Scene::registerSystem(std::unique_ptr<System> system)
{
    m_systems.push_back(std::move(system));
}

void Scene::update(float dt)
{
    PROFILE_SCOPE("Scene::update");
    for (auto& system : m_systems) system->execute(dt);
}

bool Scene::saveToFile(const std::string& path) const
{
    nlohmann::json root;
    root["nextEntityId"] = m_nextEntityId;
    root["entities"] = nlohmann::json::array();

    std::unordered_set<Entity> allEntities;
    for (const auto& [type, pool] : m_pools)
    {
        for (Entity e : pool->entities())
        {
            allEntities.insert(e);
        }
    }

    for (Entity entity : allEntities)
    {
        nlohmann::json entityJson;
        entityJson["id"] = entity.id();
        entityJson["components"] = nlohmann::json::array();

        for (const auto& [type, pool] : m_pools)
        {
            if (const ISavable* savable = pool->getSavable(entity))
            {
                entityJson["components"].push_back(savable->toJson());
            }
        }

        root["entities"].push_back(entityJson);
    }

    std::ofstream file(path);
    if (!file.is_open())
    {
        return false;
    }
    file << root.dump(2);
    return true;
}

bool Scene::loadFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }

    try
    {
        nlohmann::json root;
        file >> root;

        Scene loaded;
        loaded.m_nextEntityId = root.at("nextEntityId").get<Entity::Id>();

        for (const auto& entityJson : root.at("entities"))
        {
            Entity entity(entityJson.at("id").get<Entity::Id>());

            for (const auto& componentJson : entityJson.at("components"))
            {
                const std::string type =
                    componentJson.at("type").get<std::string>();

                const auto& registry = getComponentRegistry();
                auto it = registry.find(type);
                if (it == registry.end())
                {
                    return false;  // unknown component type
                }

                // fromJson builds the component; makePool builds its pool
                // the first time this type appears in the file. The target
                // pool is found via the registry entry's precomputed
                // typeId, not typeid(*component) - RTTI is disallowed, and
                // this is actually simpler: no runtime type inspection
                // needed at all, the string "type" already told us which
                // registry entry (and therefore which typeId) applies.
                std::unique_ptr<Component> component =
                    it->second.fromJson(componentJson);
                std::unique_ptr<IComponentPool>& poolPtr =
                    loaded.m_pools[it->second.typeId];
                if (!poolPtr)
                {
                    poolPtr = it->second.makePool();
                }
                poolPtr->insertFromBase(entity, std::move(component));
            }
        }

        m_pools = std::move(loaded.m_pools);
        m_nextEntityId = loaded.m_nextEntityId;
        return true;
    }
    catch (const nlohmann::json::exception&)
    {
        return false;
    }
}

size_t Scene::memoryBytes() const
{
    size_t bytes = 0;
    for (const auto& [type, pool] : m_pools)
    {
        bytes += pool->memoryBytes();
    }
    return bytes;
}
