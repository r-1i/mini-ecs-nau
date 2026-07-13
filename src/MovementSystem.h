#pragma once
#include "Scene.h"
#include "System.h"
#include "TransformComponent.h"

class MovementSystem : public System {
public:
	Vec3 velocity = { 1.0f, 0.0f, 0.0f };

	using System::System;
	void Execute(float dt) override {
		for (auto& [entity, transform] :
			scene_.GetEntitiesWith<TransformComponent>()) {
			transform.position.x += velocity.x * dt;
			transform.position.y += velocity.y * dt;
			transform.position.z += velocity.z * dt;
		}
	}
};