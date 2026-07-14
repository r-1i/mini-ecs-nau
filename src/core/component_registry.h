#pragma once

#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "component.h"
#include "i_component_pool.h"
#include "type_id.h"

/**
 * @brief Per-type factories, keyed by the "type" string ISavable::toJson
 * writes.
 *
 * fromJson rebuilds the component; makePool builds an empty pool for a
 * type loadFromFile may see before any addComponent<T> call ever
 * created one; typeId is precomputed here (via getTypeId<T> at
 * registration time) so loadFromFile never needs typeid() on a runtime
 * Component* to find the right pool - RTTI is disallowed.
 */
struct ComponentRegistryEntry
{
    std::function<std::unique_ptr<Component>(const nlohmann::json&)> fromJson;
    std::function<std::unique_ptr<IComponentPool>()> makePool;
    TypeId typeId;
};

/**
 * @brief Returns the registry of every known component type.
 * @return Map from the JSON "type" string to its factories.
 *
 * Defined once in component_registry.cpp - the body lives in a single
 * translation unit, so a plain function-local static is enough, no
 * header-only construct-on-first-use trick needed. Deliberately
 * doesn't include scene.h - scene.h includes this header, so the
 * reverse would be circular; routing pool creation through
 * IComponentPool avoids that.
 */
const std::unordered_map<std::string, ComponentRegistryEntry>&
getComponentRegistry();
