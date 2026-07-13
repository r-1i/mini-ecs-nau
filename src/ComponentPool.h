#pragma once
#include <cstddef>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "Component.h"
#include "Entity.h"
#include "IComponentPool.h"

// Sparse-set storage for one component type T. dense_ holds the
// actual data contiguously, so GetEntitiesWith iterates it cache-
// friendly; sparse_[Entity::id()] gives the index in dense_ directly,
// no hashing. Full rationale: docs/adr/0001-component-pool-storage.md.
template <typename T>
class ComponentPool : public IComponentPool {
  static_assert(std::is_base_of_v<Component, T>);

 public:
  template <typename... Args>
  T& Add(Entity e, Args&&... args) {
    if (e.id() >= sparse_.size()) {
      sparse_.resize(e.id() + 1, kInvalid);
    }
    sparse_[e.id()] = dense_.size();
    denseToEntity_.push_back(e);
    dense_.emplace_back(std::forward<Args>(args)...);
    return dense_.back();
  }

  bool Has(Entity e) const {
    return e.id() < sparse_.size() && sparse_[e.id()] != kInvalid;
  }

  T* Get(Entity e) { return Has(e) ? &dense_[sparse_[e.id()]] : nullptr; }

  Component* GetBase(Entity e) override { return Get(e); }

  // Swap-and-pop keeps dense_ contiguous. The idx != last guard
  // matters: removing the last (or only) element must skip the move
  // step, otherwise the fix-up below would read a denseToEntity_
  // entry that's already been popped.
  void Remove(Entity e) override {
    if (!Has(e)) return;

    const size_t idx = sparse_[e.id()];
    const size_t last = dense_.size() - 1;

    if (idx != last) {
      dense_[idx] = std::move(dense_[last]);
      denseToEntity_[idx] = denseToEntity_[last];
      sparse_[denseToEntity_[idx].id()] = idx;
    }

    dense_.pop_back();
    denseToEntity_.pop_back();
    sparse_[e.id()] = kInvalid;
  }

  // component is kept alive (not release()'d), so its destructor
  // safely destroys the now moved-from shell when this returns.
  void InsertFromBase(Entity e, std::unique_ptr<Component> component) override {
    Add(e, std::move(*static_cast<T*>(component.get())));
  }

  std::span<T> Data() { return dense_; }
  const std::vector<Entity>& Entities() const override {
    return denseToEntity_;
  }
  size_t Size() const { return dense_.size(); }

  size_t MemoryBytes() const override {
    return dense_.capacity() * sizeof(T) +
           denseToEntity_.capacity() * sizeof(Entity) +
           sparse_.capacity() * sizeof(size_t);
  }

 private:
  std::vector<T> dense_;
  std::vector<Entity> denseToEntity_;
  std::vector<size_t> sparse_;
  static constexpr size_t kInvalid = static_cast<size_t>(-1);
};
