#include <cassert>
#include <iostream>
#include <memory>

#include "../src/HealthComponent.h"
#include "../src/MovementSystem.h"
#include "../src/Scene.h"
#include "../src/TransformComponent.h"

int main() {
  Scene scene;
  Entity e1 = scene.CreateEntity();
  Entity e2 = scene.CreateEntity();
  assert(e1.id() != e2.id());

  scene.AddComponent<TransformComponent>(e1);
  scene.AddComponent<HealthComponent>(e1, 100.f);

  assert(scene.HasComponent<TransformComponent>(e1));
  assert(!scene.HasComponent<TransformComponent>(e2));

  scene.GetComponent<TransformComponent>(e1)->position.x = 5.f;
  assert(scene.GetComponent<TransformComponent>(e1)->position.x == 5.f);

  scene.DestroyEntity(e1);
  assert(!scene.HasComponent<TransformComponent>(e1));

  bool threw = false;
  try {
    scene.GetComponent<TransformComponent>(e2);
  } catch (const std::out_of_range&) {
    threw = true;
  }
  assert(threw);

  Entity e3 = scene.CreateEntity();
  scene.AddComponent<TransformComponent>(e3);

  scene.RegisterSystem(std::make_unique<MovementSystem>(scene));
  std::cout << "Pos.x [e3]: "
            << scene.GetComponent<TransformComponent>(e3)->position.x
            << std::endl;
  scene.Update(1.0f);  // Move entities for 1 second
  std::cout << "Pos.x [e3]: "
            << scene.GetComponent<TransformComponent>(e3)->position.x
            << std::endl;
  scene.Update(1.0f);  // Move entities for 1 second
  std::cout << "Pos.x [e3]: "
            << scene.GetComponent<TransformComponent>(e3)->position.x
            << std::endl;

  std::cout << "ALL SCENE TESTS PASSED\n";
  return 0;
}
