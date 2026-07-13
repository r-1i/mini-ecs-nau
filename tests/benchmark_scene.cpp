#include <iostream>
#include <memory>

#include "../src/HealthComponent.h"
#include "../src/MovementSystem.h"
#include "../src/Profiler.h"
#include "../src/Scene.h"
#include "../src/TransformComponent.h"

// Not a correctness test - this is the baseline measurement for issue 10.
// Creates a scene at a size large enough for cache-locality effects to
// show up in Scene::Update timings, then reports both timing (via
// Profiler) and an approximate memory footprint (via Scene::MemoryBytes).
// Re-run after the component-pool storage refactor to get "after" numbers
// for the same workload.
int main() {
  constexpr size_t kEntityCount = 100000;
  constexpr int kUpdateCalls = 200;

  Scene scene;
  {
    PROFILE_SCOPE("Populate scene");
    for (size_t i = 0; i < kEntityCount; ++i) {
      Entity e = scene.CreateEntity();
      scene.AddComponent<TransformComponent>(e);
      if (i % 2 == 0) {
        scene.AddComponent<HealthComponent>(e, 100.f);
      }
    }
  }

  scene.RegisterSystem(std::make_unique<MovementSystem>(scene));

  {
    PROFILE_SCOPE("Update loop (total)");
    for (int i = 0; i < kUpdateCalls; ++i) {
      scene.Update(0.016f);
    }
  }

  const size_t bytes = scene.MemoryBytes();
  std::cout << "Entities: " << kEntityCount << "\n";
  std::cout << "Update() calls: " << kUpdateCalls << "\n";
  std::cout << "Approx memory: " << bytes << " bytes ("
            << (bytes / 1024.0 / 1024.0) << " MB)\n";

  Profiler::PrintReport();
  return 0;
}
