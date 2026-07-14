#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "component.h"
#include "component_pool.h"
#include "entity.h"
#include "i_component_pool.h"
#include "system.h"
#include "type_id.h"

class Scene
{
public:
    /**
     * @brief Creates a new entity with no components.
     * @return The newly created entity.
     */
    Entity createEntity();

    /**
     * @brief Removes all components of the entity from every pool.
     * @param e Entity to destroy.
     *
     * remove() is a no-op on pools that never had this entity, so this
     * doesn't need to know which components e actually has.
     */
    void destroyEntity(Entity e);

    /**
     * @brief Attaches a component of type T to the entity, constructed
     * in place.
     * @tparam T Component type to attach.
     * @param e Target entity.
     * @param args Arguments forwarded to T's constructor.
     * @return Reference to the newly constructed component.
     */
    template <typename T, typename... Args>
    T& addComponent(Entity e, Args&&... args)
    {
        return getOrCreatePool<T>().add(e, std::forward<Args>(args)...);
    }

    /**
     * @brief Returns the entity's component of type T.
     * @tparam T Component type to look up.
     * @param e Entity to query.
     * @return Pointer to the component.
     * @throw std::out_of_range if e has no component of type T.
     */
    template <typename T>
    T* getComponent(Entity e)
    {
        ComponentPool<T>* pool = findPool<T>();
        T* component = pool ? pool->get(e) : nullptr;
        if (!component)
        {
            throw std::out_of_range("Component not found in entity");
        }
        return component;
    }

    /**
     * @brief Checks whether the entity has a component of type T.
     * @tparam T Component type to check for.
     * @param e Entity to query.
     * @return True if present.
     */
    template <typename T>
    bool hasComponent(Entity e) const
    {
        ComponentPool<T>* pool = findPool<T>();
        return pool && pool->has(e);
    }

    /**
     * @brief Returns every entity that has all of Ts..., with references
     * to those components.
     * @tparam Ts Component types the entity must have.
     * @return Entity plus live component references - mutating a
     * tuple's fields mutates the real components.
     *
     * Iterates the pool of the first type in Ts (contiguous, cache-
     * friendly) and filters the rest via hasComponent; every call site
     * today queries a single type, so picking the smallest pool among
     * several wasn't worth the extra complexity yet.
     */
    template <typename... Ts>
    std::vector<std::tuple<Entity, Ts&...>> getEntitiesWith()
    {
        using First = std::tuple_element_t<0, std::tuple<Ts...>>;
        std::vector<std::tuple<Entity, Ts&...>> result;
        ComponentPool<First>* pool = findPool<First>();
        if (!pool)
        {
            return result;
        }
        for (Entity entity : pool->entities())
        {
            if ((hasComponent<Ts>(entity) && ...))
            {
                result.emplace_back(entity, *getComponent<Ts>(entity)...);
            }
        }
        return result;
    }

    /**
     * @brief Registers a system to be updated on every Scene::update call.
     * @param system System to take ownership of.
     */
    void registerSystem(std::unique_ptr<System> system);

    /**
     * @brief Advances every registered system by dt.
     * @param dt Time step in seconds.
     */
    void update(float dt);

    /**
     * @brief Writes every entity with at least one component to a JSON file.
     * @param path Output file path.
     * @return True on success, false if the file couldn't be opened.
     *
     * Any entity that owns at least one component is written, even if
     * none of them implement ISavable - only entities with zero
     * components anywhere are skipped. getSavable resolves ISavable
     * support at compile time inside each pool (no dynamic_cast - RTTI
     * is disallowed).
     */
    bool saveToFile(const std::string& path) const;

    /**
     * @brief Replaces this Scene's contents with what's stored in path.
     * @param path Input file path.
     * @return True on success. False (this Scene left untouched) if the
     * file is missing, malformed, or references an unknown component
     * type.
     *
     * Parses into a temporary Scene and only commits on full success.
     * m_systems is left alone - loading a scene shouldn't unregister
     * systems set up beforehand.
     */
    bool loadFromFile(const std::string& path);

    /**
     * @brief Approximate memory used by all component pools combined.
     * @return Sum of every pool's own memoryBytes() - real vector
     * capacities, not the old map-node-overhead estimate. See
     * docs/adr/0001-component-pool-storage.md for before/after numbers.
     */
    size_t memoryBytes() const;

private:
    // One pool per component type, type-erased so different concrete
    // types share this map. TypeId (not std::type_index) keys it -
    // RTTI is disallowed, see type_id.h. Rationale for m_pools
    // replacing the old per-entity
    // unordered_map<type_index, unique_ptr<Component>> design:
    // docs/adr/0001-component-pool-storage.md
    std::unordered_map<TypeId, std::unique_ptr<IComponentPool>> m_pools;
    Entity::Id m_nextEntityId = 0;

    std::vector<std::unique_ptr<System>> m_systems;

    // Creates T's pool on first use. Only addComponent should call
    // this - hasComponent/getComponent/getEntitiesWith use findPool
    // instead, so querying a type that was never added doesn't
    // allocate an empty pool as a side effect.
    template <typename T>
    ComponentPool<T>& getOrCreatePool()
    {
        std::unique_ptr<IComponentPool>& poolPtr = m_pools[getTypeId<T>()];
        if (!poolPtr)
        {
            poolPtr = std::make_unique<ComponentPool<T>>();
        }
        return static_cast<ComponentPool<T>&>(*poolPtr);
    }

    // nullptr if T has no pool yet, never creates one. static_cast is
    // safe: the TypeId key this was looked up under guarantees the
    // stored pool really is a ComponentPool<T>.
    template <typename T>
    ComponentPool<T>* findPool() const
    {
        auto it = m_pools.find(getTypeId<T>());
        if (it == m_pools.end())
        {
            return nullptr;
        }
        return static_cast<ComponentPool<T>*>(it->second.get());
    }
};
