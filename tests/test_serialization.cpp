#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../src/components/health_component.h"
#include "../src/components/transform_component.h"
#include "../src/core/scene.h"
#include "../src/systems/movement_system.h"

int main()
{
    const std::string path = "test_serialization_output.json";

    // --- Round trip: save, then load into a fresh Scene ---
    Scene original;
    Entity e1 = original.createEntity();  // id 0
    original.addComponent<TransformComponent>(e1).position = {1.f, 2.f, 3.f};
    original.addComponent<HealthComponent>(e1, 100.f).takeDamage(30.f);

    Entity e2 = original.createEntity();  // id 1
    original.addComponent<TransformComponent>(e2);

    Entity e3 = original.createEntity();  // id 2, no components

    assert(original.saveToFile(path));

    Scene loaded;
    assert(loaded.loadFromFile(path));

    assert(loaded.hasComponent<TransformComponent>(e1));
    assert(loaded.getComponent<TransformComponent>(e1)->position.x == 1.f);
    assert(loaded.hasComponent<HealthComponent>(e1));
    assert(loaded.getComponent<HealthComponent>(e1)->current() == 70.f);
    assert(loaded.getComponent<HealthComponent>(e1)->max() == 100.f);

    assert(loaded.hasComponent<TransformComponent>(e2));
    assert(!loaded.hasComponent<HealthComponent>(e2));

    assert(!loaded.hasComponent<TransformComponent>(e3));

    // --- nextEntityId_ restored correctly: new entity continues past e3 ---
    Entity fresh = loaded.createEntity();
    assert(fresh.id() == 3);

    // --- loadFromFile must not wipe systems registered before the load ---
    Scene sceneWithSystem;
    sceneWithSystem.registerSystem(
        std::make_unique<MovementSystem>(sceneWithSystem));
    Entity moving = sceneWithSystem.createEntity();
    sceneWithSystem.addComponent<TransformComponent>(moving);
    assert(sceneWithSystem.loadFromFile(path));
    sceneWithSystem.update(1.0f);
    assert(
        sceneWithSystem.getComponent<TransformComponent>(moving)->position.x !=
        0.f);

    // --- Missing file ---
    Scene missing;
    assert(!missing.loadFromFile("no_such_file.json"));

    // --- Malformed JSON ---
    {
        std::ofstream bad("test_serialization_bad.json");
        bad << "{ this is not valid json !!! ";
    }
    Scene corrupt;
    assert(!corrupt.loadFromFile("test_serialization_bad.json"));

    // --- Unknown component type ---
    {
        std::ofstream unknown("test_serialization_unknown.json");
        unknown
            << R"({"nextEntityId": 1, "entities": [{"id": 0, "components": )"
               R"([{"type": "NoSuchComponent"}]}]})";
    }
    Scene badType;
    assert(!badType.loadFromFile("test_serialization_unknown.json"));

    std::remove(path.c_str());
    std::remove("test_serialization_bad.json");
    std::remove("test_serialization_unknown.json");

    std::cout << "ALL SERIALIZATION TESTS PASSED\n";
    return 0;
}
