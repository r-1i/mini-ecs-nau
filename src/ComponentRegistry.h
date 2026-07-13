#pragma once

#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "Component.h"
#include "HealthComponent.h"
#include "TransformComponent.h"

using ComponentFactory =
    std::function<std::unique_ptr<Component>(const nlohmann::json&)>;

// Maps the "type" string written by ISavable::ToJson back to a
// function that reconstructs that component from JSON. Every
// ISavable component needs an entry here - see the round-trip test
// in tests/test_serialization.cpp, which is what actually catches a
// forgotten registration.
inline const std::unordered_map<std::string, ComponentFactory>&
GetComponentRegistry() {
  static const std::unordered_map<std::string, ComponentFactory> registry = {
      {"TransformComponent", &TransformComponent::FromJson},
      {"HealthComponent", &HealthComponent::FromJson},
  };
  return registry;
}
