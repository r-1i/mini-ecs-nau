#pragma once

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Component.h"
#include "ComponentRegistry.h"
#include "Entity.h"
#include "ISavable.h"
#include "Profiler.h"
#include "System.h"

class Scene {
 public:
  Entity CreateEntity() { return Entity(nextEntityId_++); }
  void DestroyEntity(Entity e) { entities_.erase(e); }

  template <typename T, typename... Args>
  T& AddComponent(Entity e, Args&&... args) {
    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    T& ref = *component;
    entities_[e][std::type_index(typeid(T))] = std::move(component);
    // sizeof(T) is only known here, at the call site where T is a
    // concrete type - by the time components are stored as
    // Component*, that information is erased. This is what
    // MemoryBytes() below reports as real component payload size.
    totalComponentBytes_ += sizeof(T);
    return ref;
  }

  template <typename T>
  T* GetComponent(Entity e) {
    if (!HasComponent<T>(e)) {
      throw std::out_of_range("Component not found in entity");
    }
    auto& components = entities_.find(e)->second;

    // static_cast (not dynamic_cast) is safe here: the type_index key
    // this was looked up under guarantees the stored Component is
    // actually a T.
    return static_cast<T*>(
        components.find(std::type_index(typeid(T)))->second.get());
  }

  template <typename T>
  bool HasComponent(Entity e) const {
    auto entityIt = entities_.find(e);
    if (entityIt == entities_.end()) return false;
    const auto& components = entityIt->second;
    return components.find(std::type_index(typeid(T))) != components.end();
  }

  // The fold expression expands to
  // HasComponent<T1>(e) && HasComponent<T2>(e) && ...,
  // so an entity only matches if it has every requested component type.
  // Returning tuples of references (not just Entity) lets calling
  // systems read and mutate components directly, without a second
  // lookup through GetComponent.
  template <typename... Ts>
  std::vector<std::tuple<Entity, Ts&...>> GetEntitiesWith() {
    std::vector<std::tuple<Entity, Ts&...>> result;
    for (auto& [entity, components] : entities_) {
      if ((HasComponent<Ts>(entity) && ...)) {
        result.emplace_back(entity, *GetComponent<Ts>(entity)...);
      }
    }
    return result;
  }

  // Systems are stored polymorphically and owned by Scene. Adding a
  // new system only requires registering an instance here
  // Update() and Scene itself never need to change.
  void RegisterSystem(std::unique_ptr<System> system) {
    systems_.push_back(std::move(system));
  }

  void Update(float dt) {
    PROFILE_SCOPE("Scene::Update");
    for (auto& system : systems_) system->Execute(dt);
  }

  // Iterates every entity and every ISavable component it owns.
  // Components that don't implement ISavable are silently skipped -
  // not every component is required to be persistable.
  // nextEntityId_ is stored too, so entities created after loading
  // don't collide with ids restored from the file.
  bool SaveToFile(const std::string& path) const {
    nlohmann::json root;
    root["nextEntityId"] = nextEntityId_;
    root["entities"] = nlohmann::json::array();

    for (const auto& [entity, components] : entities_) {
      nlohmann::json entityJson;
      entityJson["id"] = entity.id();
      entityJson["components"] = nlohmann::json::array();

      for (const auto& [type, component] : components) {
        if (const auto* savable = dynamic_cast<ISavable*>(component.get())) {
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

  // Reads a scene previously written by SaveToFile and replaces this
  // Scene's entities with it. Parses into a temporary Scene first, so
  // a failure partway through (bad JSON, unknown component type)
  // leaves the current scene untouched instead of half-overwritten.
  // systems_ is deliberately left alone - loading a scene shouldn't
  // unregister systems set up before the load.
  // Returns false (does not throw) on any failure: missing file,
  // malformed JSON, or an unrecognized component "type" - these are
  // expected external conditions, not programmer bugs.
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

          std::unique_ptr<Component> component = it->second(componentJson);
          // typeid(*component) resolves to the dynamic (most-derived)
          // type at runtime via RTTI - we don't know T at compile
          // time here, unlike AddComponent<T>, which is why
          // GetComponent's static_cast trick doesn't apply on this path.
          loaded.entities_[entity][std::type_index(typeid(*component))] =
              std::move(component);
        }
      }

      entities_ = std::move(loaded.entities_);
      nextEntityId_ = loaded.nextEntityId_;
      // Components inserted here go through the registry factory, not
      // AddComponent<T>, so totalComponentBytes_ isn't updated - a
      // known gap in MemoryBytes() for loaded scenes.
      return true;
    } catch (const nlohmann::json::exception&) {
      return false;
    }
  }

  // Approximate memory footprint of component storage: real bytes
  // used by component payloads (tracked in AddComponent, where T is
  // still known) plus a rough estimate of unordered_map/unique_ptr
  // overhead per stored component. Node layout is
  // implementation-defined, so this is an approximation - useful for
  // a relative before/after comparison against alternative storage
  // designs, not as an exact byte count.
  size_t MemoryBytes() const {
    constexpr size_t kNodeOverhead = 4 * sizeof(void*);
    size_t bytes = totalComponentBytes_;
    for (const auto& [entity, components] : entities_) {
      bytes += components.size() *
               (sizeof(std::type_index) + sizeof(std::unique_ptr<Component>) +
                kNodeOverhead);
    }
    bytes += entities_.size() * kNodeOverhead;
    return bytes;
  }

 private:
  // Each entity exclusively owns its components (no sharing), so
  // unique_ptr is used instead of shared_ptr
  // it expresses this ownership directly and avoids
  // shared_ptr's atomic refcounting overhead.
  // std::type_index is the inner key so components of
  // different concrete types can be stored side by side and looked
  // up by type at runtime.
  std::unordered_map<
      Entity, std::unordered_map<std::type_index, std::unique_ptr<Component>>>
      entities_;
  Entity::Id nextEntityId_ = 0;

  std::vector<std::unique_ptr<System>> systems_;
  size_t totalComponentBytes_ = 0;
};
