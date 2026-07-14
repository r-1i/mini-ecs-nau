#include <iostream>
#include <memory>

#include "components/health_component.h"
#include "components/transform_component.h"
#include "core/scene.h"
#include "systems/movement_system.h"

int main()
{
    Scene scene;

    Entity e1 = scene.createEntity();
    scene.addComponent<TransformComponent>(e1);
    scene.addComponent<HealthComponent>(e1, 100.f);

    Entity e2 = scene.createEntity();
    scene.addComponent<TransformComponent>(e2);
    scene.addComponent<HealthComponent>(e2, 50.f);

    Entity e3 = scene.createEntity();
    scene.addComponent<TransformComponent>(e3);

    Entity e4 = scene.createEntity();
    scene.addComponent<HealthComponent>(e4, 100.0f);

    scene.registerSystem(std::make_unique<MovementSystem>(scene));

    auto printEntity = [&scene](Entity e)
    {
        std::cout << "Entity #" << e.id() << " [";
        bool first = true;
        if (scene.hasComponent<TransformComponent>(e))
        {
            std::cout << "Transform";
            first = false;
        }
        if (scene.hasComponent<HealthComponent>(e))
        {
            std::cout << (first ? "" : ", ") << "Health";
        }
        std::cout << "]";
        if (scene.hasComponent<TransformComponent>(e))
        {
            std::cout << " : Pos.x = "
                      << scene.getComponent<TransformComponent>(e)->position.x;
        }
        std::cout << "\n";
    };

    std::cout << "-- Before update --\n";
    for (Entity e : {e1, e2, e3, e4}) printEntity(e);

    scene.update(1.0f);
    scene.update(1.0f);
    scene.update(1.0f);

    std::cout << "\n-- After 3 frames (dt=1.0 each) --\n";
    for (Entity e : {e1, e2, e3, e4}) printEntity(e);

    return 0;
}
