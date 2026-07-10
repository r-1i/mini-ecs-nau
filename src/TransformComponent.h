#pragma once
#include "Component.h"

struct Vec3 {
  float x = 0.f, y = 0.f, z = 0.f;
};

class TransformComponent : public Component {
 public:
  Vec3 position;
  Vec3 rotation;
  Vec3 scale{1.f, 1.f, 1.f};
};
