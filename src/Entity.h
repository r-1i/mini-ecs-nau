#pragma once
#include <cstdint>
#include <functional>

class Entity {
 public:
  using Id = uint32_t;

  explicit Entity(Id id) : id_(id) {}
  Id id() const { return id_; }

  bool operator==(const Entity& other) const { return id_ == other.id_; }

 private:
  Id id_;
};

// Entity can be used as a key later
namespace std {
template <>
struct hash<Entity> {
  size_t operator()(const Entity& e) const {
    return std::hash<Entity::Id>()(e.id());
  }
};
}  // namespace std
