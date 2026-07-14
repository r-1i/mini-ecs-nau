#include <cassert>
#include <iostream>
#include <memory>

#include "../src/components/health_component.h"
#include "../src/components/transform_component.h"
#include "../src/core/scene.h"
#include "../src/systems/movement_system.h"

int main()
{
    Scene scene;
    Entity e1 = scene.createEntity();
    Entity e2 = scene.createEntity();
    assert(e1.id() != e2.id());

    scene.addComponent<TransformComponent>(e1);
    scene.addComponent<HealthComponent>(e1, 100.f);

    assert(scene.hasComponent<TransformComponent>(e1));
    assert(!scene.hasComponent<TransformComponent>(e2));

    scene.getComponent<TransformComponent>(e1)->position.x = 5.f;
    assert(scene.getComponent<TransformComponent>(e1)->position.x == 5.f);

    scene.destroyEntity(e1);
    assert(!scene.hasComponent<TransformComponent>(e1));

    bool threw = false;
    try
    {
        scene.getComponent<TransformComponent>(e2);
    }
    catch (const std::out_of_range&)
    {
        threw = true;
    }
    assert(threw);

    Entity e3 = scene.createEntity();
    scene.addComponent<TransformComponent>(e3);

    scene.registerSystem(std::make_unique<MovementSystem>(scene));
    std::cout << "Pos.x [e3]: "
              << scene.getComponent<TransformComponent>(e3)->position.x
              << std::endl;
    scene.update(1.0f);  // Move entities for 1 second
    std::cout << "Pos.x [e3]: "
              << scene.getComponent<TransformComponent>(e3)->position.x
              << std::endl;
    scene.update(1.0f);  // Move entities for 1 second
    std::cout << "Pos.x [e3]: "
              << scene.getComponent<TransformComponent>(e3)->position.x
              << std::endl;

    std::cout << "ALL SCENE TESTS PASSED\n";
    return 0;
}
