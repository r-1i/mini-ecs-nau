#include <iostream>
#include <memory>

#include "HealthComponent.h"
#include "MovementSystem.h"
#include "Scene.h"
#include "TransformComponent.h"

int main() {
  Scene scene;

  Entity e1 = scene.CreateEntity();
  scene.AddComponent<TransformComponent>(e1);
  scene.AddComponent<HealthComponent>(e1, 100.f);

  Entity e2 = scene.CreateEntity();
  scene.AddComponent<TransformComponent>(e2);
  scene.AddComponent<HealthComponent>(e2, 50.f);

  Entity e3 = scene.CreateEntity();
  scene.AddComponent<TransformComponent>(e3);

  Entity e4 = scene.CreateEntity();
  scene.AddComponent<HealthComponent>(e4, 100.0f);

  scene.RegisterSystem(std::make_unique<MovementSystem>(scene));

  auto printEntity = [&scene](Entity e) {
    std::cout << "Entity #" << e.id() << " [";
    bool first = true;
    if (scene.HasComponent<TransformComponent>(e)) {
      std::cout << "Transform";
      first = false;
    }
    if (scene.HasComponent<HealthComponent>(e)) {
      std::cout << (first ? "" : ", ") << "Health";
    }
    std::cout << "]";
    if (scene.HasComponent<TransformComponent>(e)) {
      std::cout << " : Pos.x = "
                << scene.GetComponent<TransformComponent>(e)->position.x;
    }
    std::cout << "\n";
  };

  std::cout << "-- Before update --\n";
  for (Entity e : {e1, e2, e3, e4}) printEntity(e);

  scene.Update(1.0f);
  scene.Update(1.0f);
  scene.Update(1.0f);

  std::cout << "\n-- After 3 frames (dt=1.0 each) --\n";
  for (Entity e : {e1, e2, e3, e4}) printEntity(e);

  return 0;
}
