#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../src/HealthComponent.h"
#include "../src/MovementSystem.h"
#include "../src/Scene.h"
#include "../src/TransformComponent.h"

int main() {
  const std::string path = "test_serialization_output.json";

  // --- Round trip: save, then load into a fresh Scene ---
  Scene original;
  Entity e1 = original.CreateEntity();  // id 0
  original.AddComponent<TransformComponent>(e1).position = {1.f, 2.f, 3.f};
  original.AddComponent<HealthComponent>(e1, 100.f).takeDamage(30.f);

  Entity e2 = original.CreateEntity();  // id 1
  original.AddComponent<TransformComponent>(e2);

  Entity e3 = original.CreateEntity();  // id 2, no components

  assert(original.SaveToFile(path));

  Scene loaded;
  assert(loaded.LoadFromFile(path));

  assert(loaded.HasComponent<TransformComponent>(e1));
  assert(loaded.GetComponent<TransformComponent>(e1)->position.x == 1.f);
  assert(loaded.HasComponent<HealthComponent>(e1));
  assert(loaded.GetComponent<HealthComponent>(e1)->current() == 70.f);
  assert(loaded.GetComponent<HealthComponent>(e1)->max() == 100.f);

  assert(loaded.HasComponent<TransformComponent>(e2));
  assert(!loaded.HasComponent<HealthComponent>(e2));

  assert(!loaded.HasComponent<TransformComponent>(e3));

  // --- nextEntityId_ restored correctly: new entity continues past e3 ---
  Entity fresh = loaded.CreateEntity();
  assert(fresh.id() == 3);

  // --- LoadFromFile must not wipe systems registered before the load ---
  Scene sceneWithSystem;
  sceneWithSystem.RegisterSystem(
      std::make_unique<MovementSystem>(sceneWithSystem));
  Entity moving = sceneWithSystem.CreateEntity();
  sceneWithSystem.AddComponent<TransformComponent>(moving);
  assert(sceneWithSystem.LoadFromFile(path));
  sceneWithSystem.Update(1.0f);
  assert(sceneWithSystem.GetComponent<TransformComponent>(moving)->position.x !=
         0.f);

  // --- Missing file ---
  Scene missing;
  assert(!missing.LoadFromFile("no_such_file.json"));

  // --- Malformed JSON ---
  {
    std::ofstream bad("test_serialization_bad.json");
    bad << "{ this is not valid json !!! ";
  }
  Scene corrupt;
  assert(!corrupt.LoadFromFile("test_serialization_bad.json"));

  // --- Unknown component type ---
  {
    std::ofstream unknown("test_serialization_unknown.json");
    unknown << R"({"nextEntityId": 1, "entities": [{"id": 0, "components": )"
               R"([{"type": "NoSuchComponent"}]}]})";
  }
  Scene badType;
  assert(!badType.LoadFromFile("test_serialization_unknown.json"));

  std::remove(path.c_str());
  std::remove("test_serialization_bad.json");
  std::remove("test_serialization_unknown.json");

  std::cout << "ALL SERIALIZATION TESTS PASSED\n";
  return 0;
}
