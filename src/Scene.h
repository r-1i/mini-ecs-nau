#pragma once

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Component.h"
#include "ComponentPool.h"
#include "ComponentRegistry.h"
#include "Entity.h"
#include "IComponentPool.h"
#include "ISavable.h"
#include "Profiler.h"
#include "System.h"

class Scene {
 public:
  Entity CreateEntity() { return Entity(nextEntityId_++); }

  // Removes the entity from every pool. Each Remove() is a safe
  // no-op on pools that never had this entity.
  void DestroyEntity(Entity e) {
    for (auto& [type, pool] : pools_) {
      pool->Remove(e);
    }
  }

  template <typename T, typename... Args>
  T& AddComponent(Entity e, Args&&... args) {
    return GetOrCreatePool<T>().Add(e, std::forward<Args>(args)...);
  }

  // Throws std::out_of_range if e has no component of type T.
  template <typename T>
  T* GetComponent(Entity e) {
    ComponentPool<T>* pool = FindPool<T>();
    T* component = pool ? pool->Get(e) : nullptr;
    if (!component) {
      throw std::out_of_range("Component not found in entity");
    }
    return component;
  }

  template <typename T>
  bool HasComponent(Entity e) const {
    ComponentPool<T>* pool = FindPool<T>();
    return pool && pool->Has(e);
  }

  // Returns references into live component storage - mutating a
  // tuple's fields mutates the real components, no second lookup
  // needed. Iterates the pool of the first type in Ts (contiguous,
  // cache-friendly) and filters the rest via HasComponent; every
  // call site today queries a single type, so picking the smallest
  // pool among several wasn't worth the extra complexity yet.
  template <typename... Ts>
  std::vector<std::tuple<Entity, Ts&...>> GetEntitiesWith() {
    using First = std::tuple_element_t<0, std::tuple<Ts...>>;
    std::vector<std::tuple<Entity, Ts&...>> result;
    ComponentPool<First>* pool = FindPool<First>();
    if (!pool) {
      return result;
    }
    for (Entity entity : pool->Entities()) {
      if ((HasComponent<Ts>(entity) && ...)) {
        result.emplace_back(entity, *GetComponent<Ts>(entity)...);
      }
    }
    return result;
  }

  // Systems are owned by Scene; adding a new one is just another
  // RegisterSystem call, Update() itself never changes.
  void RegisterSystem(std::unique_ptr<System> system) {
    systems_.push_back(std::move(system));
  }

  void Update(float dt) {
    PROFILE_SCOPE("Scene::Update");
    for (auto& system : systems_) system->Execute(dt);
  }

  // Writes every entity that owns at least one component, even if
  // none of them implement ISavable (matches pre-refactor behavior -
  // only entities with zero components anywhere are skipped). There
  // is no separate entity list anymore, so the set of entities is
  // the union of Entities() across all pools.
  bool SaveToFile(const std::string& path) const {
    nlohmann::json root;
    root["nextEntityId"] = nextEntityId_;
    root["entities"] = nlohmann::json::array();

    std::unordered_set<Entity> allEntities;
    for (const auto& [type, pool] : pools_) {
      for (Entity e : pool->Entities()) {
        allEntities.insert(e);
      }
    }

    for (Entity entity : allEntities) {
      nlohmann::json entityJson;
      entityJson["id"] = entity.id();
      entityJson["components"] = nlohmann::json::array();

      for (const auto& [type, pool] : pools_) {
        if (const auto* savable =
                dynamic_cast<ISavable*>(pool->GetBase(entity))) {
          entityJson["components"].push_back(savable->ToJson());
        }
      }

      root["entities"].push_back(entityJson);
    }

    std::ofstream file(path);
    if (!file.is_open()) {
      return false;
    }
    file << root.dump(2);
    return true;
  }

  // Parses into a temporary Scene and only commits on full success,
  // so a bad file (missing, malformed JSON, unknown component type)
  // leaves this Scene untouched. systems_ is left alone, loading a
  // scene shouldn't unregister systems set up beforehand.
  bool LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      return false;
    }

    try {
      nlohmann::json root;
      file >> root;

      Scene loaded;
      loaded.nextEntityId_ = root.at("nextEntityId").get<Entity::Id>();

      for (const auto& entityJson : root.at("entities")) {
        Entity entity(entityJson.at("id").get<Entity::Id>());

        for (const auto& componentJson : entityJson.at("components")) {
          const std::string type = componentJson.at("type").get<std::string>();

          const auto& registry = GetComponentRegistry();
          auto it = registry.find(type);
          if (it == registry.end()) {
            return false;  // unknown component type
          }

          // fromJson builds the component; makePool builds its pool
          // the first time this type appears in the file - neither
          // needs Scene to know T.
          std::unique_ptr<Component> component =
              it->second.fromJson(componentJson);
          const std::type_index ti(typeid(*component));
          std::unique_ptr<IComponentPool>& poolPtr = loaded.pools_[ti];
          if (!poolPtr) {
            poolPtr = it->second.makePool();
          }
          poolPtr->InsertFromBase(entity, std::move(component));
        }
      }

      pools_ = std::move(loaded.pools_);
      nextEntityId_ = loaded.nextEntityId_;
      return true;
    } catch (const nlohmann::json::exception&) {
      return false;
    }
  }

  // Sums each pool's own MemoryBytes() - real vector capacities, a
  // tighter number than the old map-node-overhead estimate. See
  // docs/adr/0001-component-pool-storage.md for the before/after
  size_t MemoryBytes() const {
    size_t bytes = 0;
    for (const auto& [type, pool] : pools_) {
      bytes += pool->MemoryBytes();
    }
    return bytes;
  }

 private:
  // One pool per component type, type-erased so different concrete
  // types share this map. Rationale for replacing the old per-entity
  // unordered_map<type_index, unique_ptr<Component>>
  // design: docs/adr/0001-component-pool-storage.md
  std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> pools_;
  Entity::Id nextEntityId_ = 0;

  std::vector<std::unique_ptr<System>> systems_;

  // Creates T's pool on first use. Only AddComponent should call
  // this, HasComponent/GetComponent/GetEntitiesWith use FindPool
  // instead, so querying a type that was never added doesn't have
  // the side effect of allocating an empty pool for it.
  template <typename T>
  ComponentPool<T>& GetOrCreatePool() {
    std::unique_ptr<IComponentPool>& poolPtr =
        pools_[std::type_index(typeid(T))];
    if (!poolPtr) {
      poolPtr = std::make_unique<ComponentPool<T>>();
    }
    return static_cast<ComponentPool<T>&>(*poolPtr);
  }

  // nullptr if T has no pool yet never creates one. static_cast is
  // safe: the type_index key this was looked up under guarantees the
  // stored pool really is a ComponentPool<T>.
  template <typename T>
  ComponentPool<T>* FindPool() const {
    auto it = pools_.find(std::type_index(typeid(T)));
    if (it == pools_.end()) {
      return nullptr;
    }
    return static_cast<ComponentPool<T>*>(it->second.get());
  }
};
