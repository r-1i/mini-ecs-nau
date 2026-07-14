#pragma once
#include <cstddef>
#include <memory>
#include <vector>

#include "component.h"
#include "entity.h"
#include "i_savable.h"

/**
 * @brief Type-erased interface for one component type's storage.
 *
 * Scene holds pools of different concrete types in a single map
 * (m_pools) and needs these virtual calls only where it doesn't know
 * T: destroying an entity, save/load, and total memory usage.
 */
class IComponentPool
{
public:
    virtual ~IComponentPool() = default;

    /** @brief Removes the entity's component from this pool, if present. */
    virtual void remove(Entity e) = 0;

    /**
     * @brief Inserts a component built from a type-erased base pointer.
     * @param e Entity to attach it to.
     * @param component Component to insert; must be this pool's T.
     *
     * Used by Scene::loadFromFile to insert a component built by
     * ComponentRegistry, whose factories only know Component, not T.
     */
    virtual void insertFromBase(Entity e,
                                std::unique_ptr<Component> component) = 0;

    /**
     * @brief Returns the entity's component as ISavable, if supported.
     * @param e Entity to query.
     * @return Non-owning pointer; nullptr if absent or T doesn't
     * implement ISavable.
     *
     * No dynamic_cast: each ComponentPool<T> resolves this at compile
     * time via if constexpr, since it knows T - RTTI is disallowed by
     * the project's coding style, this is the compile-time substitute.
     */
    virtual const ISavable* getSavable(Entity e) const = 0;

    /** @brief Entities that currently have a component in this pool. */
    virtual const std::vector<Entity>& entities() const = 0;

    /** @brief Approximate memory owned by this pool's storage, in bytes. */
    virtual size_t memoryBytes() const = 0;
};
