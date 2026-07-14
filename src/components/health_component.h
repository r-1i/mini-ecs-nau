#pragma once

#include <memory>

#include "../core/component.h"
#include "../core/i_savable.h"

/**
 * @brief Current/max health with damage and heal helpers.
 */
class HealthComponent : public Component, public ISavable
{
public:
    /**
     * @brief Constructs at full health.
     * @param maxHealth Starting and max health.
     */
    explicit HealthComponent(float maxHealth);

    /**
     * @brief Constructs with an explicit current health.
     * @param currentHealth Starting health.
     * @param maxHealth Maximum health; heal() clamps to this.
     */
    explicit HealthComponent(float currentHealth, float maxHealth);

    float current() const { return m_currentHealth; }
    float max() const { return m_maxHealth; }

    /** @brief Reduces current health by amount, clamped to 0. */
    void takeDamage(float amount);

    /** @brief Increases current health by amount, clamped to max(). */
    void heal(float amount);

    /** @brief Serializes current/max health to JSON. */
    nlohmann::json toJson() const override;

    /**
     * @brief Rebuilds a HealthComponent from JSON produced by toJson().
     * @param j JSON with "currentHealth"/"maxHealth" fields.
     * @return Newly constructed component.
     */
    static std::unique_ptr<Component> fromJson(const nlohmann::json& j);

private:
    float m_maxHealth;
    float m_currentHealth;
};
