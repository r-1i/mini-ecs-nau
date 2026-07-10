#pragma once

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

#include "Component.h"
#include "Entity.h"

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

 private:
  // storage
  std::unordered_map<
      Entity, std::unordered_map<std::type_index, std::unique_ptr<Component>>>
      entities_;
  Entity::Id nextEntityId_ = 0;
};
