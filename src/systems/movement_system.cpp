#include "movement_system.h"

void MovementSystem::execute(float dt)
{
    for (auto& [entity, transform] :
         m_scene.getEntitiesWith<TransformComponent>())
    {
        transform.position.x += velocity.x * dt;
        transform.position.y += velocity.y * dt;
        transform.position.z += velocity.z * dt;
    }
}
