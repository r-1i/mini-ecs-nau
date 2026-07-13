#pragma once

class Scene;

class System {
 public:
  // Scene is injected once through the constructor instead of being
  // passed to Execute() on every call, and instead of using a Scene
  // singleton, which would make Scene harder to test in isolation.
  explicit System(Scene& scene) : scene_(scene) {}
  virtual ~System() = default;
  virtual void Execute(float dt) = 0;

 protected:
  Scene& scene_;
};
