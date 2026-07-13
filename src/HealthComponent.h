#pragma once
#include <algorithm>
#include <memory>

#include "Component.h"
#include "ISavable.h"

class HealthComponent : public Component, public ISavable {
 public:
  explicit HealthComponent(float maxHealth)
      : maxHealth_(maxHealth), currentHealth_(maxHealth) {}

  explicit HealthComponent(float currentHealth, float maxHealth)
      : maxHealth_(maxHealth), currentHealth_(currentHealth) {}

  float current() const { return currentHealth_; }
  float max() const { return maxHealth_; }

  void takeDamage(float amount) {
    currentHealth_ = std::max(0.f, currentHealth_ - amount);
  }
  void heal(float amount) {
    currentHealth_ = std::min(maxHealth_, currentHealth_ + amount);
  }

  nlohmann::json ToJson() const override {
    return {
        {"type", "HealthComponent"},
        {"currentHealth", currentHealth_},
        {"maxHealth", maxHealth_},
    };
  }

  static std::unique_ptr<Component> FromJson(const nlohmann::json& j) {
    return std::make_unique<HealthComponent>(j["currentHealth"].get<float>(),
                                             j["maxHealth"].get<float>());
  }

 private:
  float maxHealth_;
  float currentHealth_;
};
