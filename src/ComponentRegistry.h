#pragma once

#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "Component.h"
#include "ComponentPool.h"
#include "HealthComponent.h"
#include "IComponentPool.h"
#include "TransformComponent.h"

// Per-type factories, keyed by the "type" string ISavable::ToJson
// writes. fromJson rebuilds the component; makePool builds an empty
// pool for a type LoadFromFile may see before any AddComponent<T>
// call ever created one. Deliberately doesn't include Scene.h -
// Scene.h includes this header, so the reverse would be circular;
// routing pool creation through IComponentPool avoids that.
struct ComponentRegistryEntry {
  std::function<std::unique_ptr<Component>(const nlohmann::json&)> fromJson;
  std::function<std::unique_ptr<IComponentPool>()> makePool;
};

inline const std::unordered_map<std::string, ComponentRegistryEntry>&
GetComponentRegistry() {
  static const std::unordered_map<std::string, ComponentRegistryEntry> registry = {
      {"TransformComponent",
       {&TransformComponent::FromJson,
        [] { return std::make_unique<ComponentPool<TransformComponent>>(); }}},
      {"HealthComponent",
       {&HealthComponent::FromJson,
        [] { return std::make_unique<ComponentPool<HealthComponent>>(); }}},
  };
  return registry;
}
