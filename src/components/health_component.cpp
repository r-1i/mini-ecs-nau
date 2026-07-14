#include "health_component.h"

#include <algorithm>

HealthComponent::HealthComponent(float maxHealth)
    : m_maxHealth(maxHealth), m_currentHealth(maxHealth)
{
}

HealthComponent::HealthComponent(float currentHealth, float maxHealth)
    : m_maxHealth(maxHealth), m_currentHealth(currentHealth)
{
}

void HealthComponent::takeDamage(float amount)
{
    m_currentHealth = std::max(0.f, m_currentHealth - amount);
}

void HealthComponent::heal(float amount)
{
    m_currentHealth = std::min(m_maxHealth, m_currentHealth + amount);
}

nlohmann::json HealthComponent::toJson() const
{
    return {
        {"type", "HealthComponent"},
        {"currentHealth", m_currentHealth},
        {"maxHealth", m_maxHealth},
    };
}

std::unique_ptr<Component> HealthComponent::fromJson(const nlohmann::json& j)
{
    return std::make_unique<HealthComponent>(j["currentHealth"].get<float>(),
                                             j["maxHealth"].get<float>());
}
