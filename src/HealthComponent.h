#pragma once
#include <algorithm>

#include "Component.h"

class HealthComponent : public Component {
 public:
  explicit HealthComponent(float maxHealth)
      : maxHealth_(maxHealth), currentHealth_(maxHealth) {}

  float current() const { return currentHealth_; }
  float max() const { return maxHealth_; }

  void takeDamage(float amount) {
    currentHealth_ = std::max(0.f, currentHealth_ - amount);
  }
  void heal(float amount) {
    currentHealth_ = std::min(maxHealth_, currentHealth_ + amount);
  }

 private:
  float maxHealth_;
  float currentHealth_;
};
