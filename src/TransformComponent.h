#pragma once
#include "Component.h"
#include "ISavable.h"

struct Vec3 {
  float x = 0.f, y = 0.f, z = 0.f;
};

class TransformComponent : public Component, public ISavable {
 public:
  Vec3 position;
  Vec3 rotation;
  Vec3 scale{1.f, 1.f, 1.f};

  nlohmann::json ToJson() const override {
    return {{"type", "TransformComponent"},
            {"position", {position.x, position.y, position.z}},
            {"rotation", {rotation.x, rotation.y, rotation.z}},
            {"scale", {scale.x, scale.y, scale.z}}};
  }

  static std::unique_ptr<Component> FromJson(const nlohmann::json& j) {
    auto transform = std::make_unique<TransformComponent>();
    transform->position = {j["position"][0], j["position"][1],
                           j["position"][2]};
    transform->rotation = {j["rotation"][0], j["rotation"][1],
                           j["rotation"][2]};
    transform->scale = {j["scale"][0], j["scale"][1], j["scale"][2]};
    return transform;
  }
};
