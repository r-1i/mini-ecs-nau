#pragma once

#include <memory>
#include <stdexcept>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Component.h"
#include "Entity.h"
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
    for (auto& system : systems_) system->Execute(dt);
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
};
