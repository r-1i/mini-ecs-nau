#include <cassert>
#include <iostream>
#include <memory>

#include "../src/MovementSystem.h"
#include "../src/Scene.h"
#include "../src/TransformComponent.h"

int main() {
  Scene scene;

  Entity moving = scene.CreateEntity();
  scene.AddComponent<TransformComponent>(moving);

  Entity still = scene.CreateEntity();

  auto movement = std::make_unique<MovementSystem>(scene);
  movement->velocity = {1.0f, 2.0f, 0.0f};
  scene.RegisterSystem(std::move(movement));

  scene.Update(1.0f);
  scene.Update(0.5f);  // dt = 1.5

  auto* transform = scene.GetComponent<TransformComponent>(moving);
  assert(transform->position.x == 1.5f);
  assert(transform->position.y == 3.0f);
  assert(transform->position.z == 0.0f);

  // entity without TransformComponent musn't brake the system
  assert(!scene.HasComponent<TransformComponent>(still));

  std::cout << "ALL MOVEMENT SYSTEM TESTS PASSED\n";
  return 0;
}
