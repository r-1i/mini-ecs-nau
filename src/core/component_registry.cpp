#include "component_registry.h"

#include "../components/health_component.h"
#include "../components/transform_component.h"
#include "component_pool.h"

const std::unordered_map<std::string, ComponentRegistryEntry>&
getComponentRegistry()
{
    static const std::unordered_map<std::string, ComponentRegistryEntry>
        registry = {
            {"TransformComponent",
             {&TransformComponent::fromJson, []
              { return std::make_unique<ComponentPool<TransformComponent>>(); },
              getTypeId<TransformComponent>()}},
            {"HealthComponent",
             {&HealthComponent::fromJson,
              [] { return std::make_unique<ComponentPool<HealthComponent>>(); },
              getTypeId<HealthComponent>()}},
        };
    return registry;
}
