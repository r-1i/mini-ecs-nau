#pragma once

#include "../components/transform_component.h"
#include "../core/scene.h"
#include "../core/system.h"

/**
 * @brief Moves every entity with a TransformComponent by velocity * dt.
 */
class MovementSystem : public System
{
public:
    Vec3 velocity = {1.0f, 0.0f, 0.0f};

    using System::System;

    /** @brief Applies velocity * dt to every entity with a TransformComponent.
     */
    void execute(float dt) override;
};
