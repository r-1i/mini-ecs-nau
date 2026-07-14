#include "transform_component.h"

nlohmann::json TransformComponent::toJson() const
{
    return {{"type", "TransformComponent"},
            {"position", {position.x, position.y, position.z}},
            {"rotation", {rotation.x, rotation.y, rotation.z}},
            {"scale", {scale.x, scale.y, scale.z}}};
}

std::unique_ptr<Component> TransformComponent::fromJson(const nlohmann::json& j)
{
    auto transform = std::make_unique<TransformComponent>();
    transform->position = {j["position"][0], j["position"][1],
                           j["position"][2]};
    transform->rotation = {j["rotation"][0], j["rotation"][1],
                           j["rotation"][2]};
    transform->scale = {j["scale"][0], j["scale"][1], j["scale"][2]};
    return transform;
}
