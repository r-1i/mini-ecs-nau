#include <cassert>
#include <iostream>

#include "../src/HealthComponent.h"
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

  std::cout << "ALL SCENE TESTS PASSED\n";
  return 0;
}
