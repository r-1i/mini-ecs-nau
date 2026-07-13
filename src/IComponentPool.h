#pragma once
#include <cstddef>
#include <memory>
#include <vector>

#include "Component.h"
#include "Entity.h"

// Type-erased interface for one component type's storage. Scene
// holds pools of different concrete types in a single map (pools_)
// and needs these virtual calls only where it doesn't know T:
// destroying an entity, save/load, and total memory usage.
class IComponentPool {
 public:
  virtual ~IComponentPool() = default;
  virtual void Remove(Entity e) = 0;
  // Non-owning; nullptr if the entity has no component in this pool.
  virtual Component* GetBase(Entity e) = 0;
  // Used by Scene::LoadFromFile to insert a component built by
  // ComponentRegistry, whose factories only know Component, not T.
  virtual void InsertFromBase(Entity e, std::unique_ptr<Component> component) = 0;
  virtual const std::vector<Entity>& Entities() const = 0;
  virtual size_t MemoryBytes() const = 0;
};
