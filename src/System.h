#pragma once

class Scene;

class System {
public:
	explicit System(Scene& scene) : scene_(scene) {}
	virtual ~System() = default;
	virtual void Execute(float dt) = 0;

protected:
	Scene& scene_;
};