#include "../src/components/health_component.h"
#include "../src/components/transform_component.h"
#include "../src/core/scene.h"
#include "../src/systems/movement_system.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

int main()
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    Scene scene;

    for (int i = 0; i < 1000; ++i)
    {
        Entity e = scene.createEntity();
        scene.addComponent<TransformComponent>(e);
        scene.addComponent<HealthComponent>(e, 100.f);
        scene.destroyEntity(e);
    }
}
