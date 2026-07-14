#include <iostream>
#include <memory>

#include "../src/components/health_component.h"
#include "../src/components/transform_component.h"
#include "../src/core/scene.h"
#include "../src/profiling/profiler.h"
#include "../src/systems/movement_system.h"

// Not a correctness test - this is the baseline measurement for issue 10.
// Creates a scene at a size large enough for cache-locality effects to
// show up in Scene::update timings, then reports both timing (via
// Profiler) and an approximate memory footprint (via Scene::memoryBytes).
// Re-run after the component-pool storage refactor to get "after" numbers
// for the same workload.
int main()
{
    constexpr size_t ENTITY_COUNT = 100000;
    constexpr int UPDATE_CALLS = 200;

    Scene scene;
    {
        PROFILE_SCOPE("Populate scene");
        for (size_t i = 0; i < ENTITY_COUNT; ++i)
        {
            Entity e = scene.createEntity();
            scene.addComponent<TransformComponent>(e);
            if (i % 2 == 0)
            {
                scene.addComponent<HealthComponent>(e, 100.f);
            }
        }
    }

    scene.registerSystem(std::make_unique<MovementSystem>(scene));

    {
        PROFILE_SCOPE("Update loop (total)");
        for (int i = 0; i < UPDATE_CALLS; ++i)
        {
            scene.update(0.016f);
        }
    }

    const size_t bytes = scene.memoryBytes();
    std::cout << "Entities: " << ENTITY_COUNT << "\n";
    std::cout << "Update() calls: " << UPDATE_CALLS << "\n";
    std::cout << "Approx memory: " << bytes << " bytes ("
              << (bytes / 1024.0 / 1024.0) << " MB)\n";

    Profiler::printReport();
    return 0;
}
