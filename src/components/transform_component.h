#pragma once

#include <memory>

#include "../core/component.h"
#include "../core/i_savable.h"

/// Plain 3-float vector; storage only for position/rotation/scale.
struct Vec3
{
    float x = 0.f, y = 0.f, z = 0.f;
};

/**
 * @brief Position, rotation and scale of an entity.
 */
class TransformComponent : public Component, public ISavable
{
public:
    Vec3 position;
    Vec3 rotation;
    Vec3 scale{1.f, 1.f, 1.f};

    /** @brief Serializes position/rotation/scale to JSON. */
    nlohmann::json toJson() const override;

    /**
     * @brief Rebuilds a TransformComponent from JSON produced by toJson().
     * @param j JSON object with "position"/"rotation"/"scale" arrays.
     * @return Newly constructed component.
     */
    static std::unique_ptr<Component> fromJson(const nlohmann::json& j);
};
