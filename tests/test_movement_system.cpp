#include <cassert>
#include <iostream>
#include <memory>

#include "../src/components/transform_component.h"
#include "../src/core/scene.h"
#include "../src/systems/movement_system.h"

int main()
{
    Scene scene;

    Entity moving = scene.createEntity();
    scene.addComponent<TransformComponent>(moving);

    Entity still = scene.createEntity();

    auto movement = std::make_unique<MovementSystem>(scene);
    movement->velocity = {1.0f, 2.0f, 0.0f};
    scene.registerSystem(std::move(movement));

    scene.update(1.0f);
    scene.update(0.5f);  // dt = 1.5

    auto* transform = scene.getComponent<TransformComponent>(moving);
    assert(transform->position.x == 1.5f);
    assert(transform->position.y == 3.0f);
    assert(transform->position.z == 0.0f);

    // entity without TransformComponent musn't brake the system
    assert(!scene.hasComponent<TransformComponent>(still));

    std::cout << "ALL MOVEMENT SYSTEM TESTS PASSED\n";
    return 0;
}
