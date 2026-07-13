#pragma once
#include <cstdint>
#include <functional>

class Entity {
 public:
  using Id = uint32_t;

  // explicit prevents accidental implicit conversion from a raw uint32_t
  // (e.g. an array index) into an Entity, avoiding silent mix-ups
  // between entity ids and unrelated integers.
  explicit Entity(Id id) : id_(id) {}
  Id id() const { return id_; }

  bool operator==(const Entity& other) const { return id_ == other.id_; }

 private:
  Id id_;
};

namespace std {
template <>
// Specializing std::hash lets Entity be used as a key in
// std::unordered_map/unordered_set, which Scene's storage relies on.
struct hash<Entity> {
  size_t operator()(const Entity& e) const {
    return std::hash<Entity::Id>()(e.id());
  }
};
}  // namespace std
