#pragma once
#include <cstddef>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "component.h"
#include "entity.h"
#include "i_component_pool.h"
#include "i_savable.h"

/**
 * @brief Sparse-set storage for one component type T.
 * @tparam T Component type stored; must derive from Component.
 *
 * m_dense holds the actual data contiguously, so getEntitiesWith
 * iterates it cache-friendly; m_sparse[Entity::id()] gives the index
 * in m_dense directly, no hashing. Full rationale:
 * docs/adr/0001-component-pool-storage.md.
 */
template <typename T>
class ComponentPool : public IComponentPool
{
    static_assert(std::is_base_of_v<Component, T>);

public:
    /**
     * @brief Constructs a T in place and attaches it to the entity.
     * @param e Entity to attach it to.
     * @param args Arguments forwarded to T's constructor.
     * @return Reference to the newly constructed component.
     */
    template <typename... Args>
    T& add(Entity e, Args&&... args)
    {
        if (e.id() >= m_sparse.size())
        {
            m_sparse.resize(e.id() + 1, INVALID_INDEX);
        }
        m_sparse[e.id()] = m_dense.size();
        m_denseToEntity.push_back(e);
        m_dense.emplace_back(std::forward<Args>(args)...);
        return m_dense.back();
    }

    /** @brief Checks whether the entity has a component in this pool. */
    bool has(Entity e) const
    {
        return e.id() < m_sparse.size() && m_sparse[e.id()] != INVALID_INDEX;
    }

    /** @brief Returns the entity's component, or nullptr if absent. */
    T* get(Entity e) { return has(e) ? &m_dense[m_sparse[e.id()]] : nullptr; }

    /**
     * @brief Returns the entity's component as ISavable.
     * @param e Entity to query.
     * @return Non-owning pointer; nullptr if absent or T doesn't
     * implement ISavable.
     *
     * if constexpr picks the branch at compile time based on whether T
     * implements ISavable - the discarded branch doesn't even need to
     * compile for the other case, so this works for both savable and
     * non-savable component types without dynamic_cast.
     */
    const ISavable* getSavable(Entity e) const override
    {
        if constexpr (std::is_base_of_v<ISavable, T>)
        {
            return has(e) ? &m_dense[m_sparse[e.id()]] : nullptr;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * @brief Removes the entity's component from this pool.
     * @param e Entity to remove.
     *
     * Swap-and-pop keeps m_dense contiguous. The idx != last guard
     * matters: removing the last (or only) element must skip the move
     * step, otherwise the fix-up below would read a m_denseToEntity
     * entry that's already been popped.
     */
    void remove(Entity e) override
    {
        if (!has(e)) return;

        const size_t idx = m_sparse[e.id()];
        const size_t last = m_dense.size() - 1;

        if (idx != last)
        {
            m_dense[idx] = std::move(m_dense[last]);
            m_denseToEntity[idx] = m_denseToEntity[last];
            m_sparse[m_denseToEntity[idx].id()] = idx;
        }

        m_dense.pop_back();
        m_denseToEntity.pop_back();
        m_sparse[e.id()] = INVALID_INDEX;
    }

    /**
     * @brief Inserts a component built from a type-erased base pointer.
     * @param e Entity to attach it to.
     * @param component Component to insert; must actually be a T.
     *
     * component is kept alive (not release()'d), so its destructor
     * safely destroys the now moved-from shell when this returns.
     */
    void insertFromBase(Entity e, std::unique_ptr<Component> component) override
    {
        add(e, std::move(*static_cast<T*>(component.get())));
    }

    /** @brief Contiguous view over every component currently stored. */
    std::span<T> data() { return m_dense; }

    /** @brief Entities that currently have a component in this pool. */
    const std::vector<Entity>& entities() const override
    {
        return m_denseToEntity;
    }

    /** @brief Number of components currently stored. */
    size_t size() const { return m_dense.size(); }

    /** @brief Approximate memory owned by this pool's storage, in bytes. */
    size_t memoryBytes() const override
    {
        return m_dense.capacity() * sizeof(T) +
               m_denseToEntity.capacity() * sizeof(Entity) +
               m_sparse.capacity() * sizeof(size_t);
    }

private:
    std::vector<T> m_dense;
    std::vector<Entity> m_denseToEntity;
    std::vector<size_t> m_sparse;
    static constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);
};
