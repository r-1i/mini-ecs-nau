#include "../src/HealthComponent.h"
#include "../src/MovementSystem.h"
#include "../src/Scene.h"
#include "../src/TransformComponent.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

int main() {
#ifdef _DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
  Scene scene;

  for (int i = 0; i < 1000; ++i) {
    Entity e = scene.CreateEntity();
    scene.AddComponent<TransformComponent>(e);
    scene.AddComponent<HealthComponent>(e, 100.f);
    scene.DestroyEntity(e);
  }
}
