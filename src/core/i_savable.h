#pragma once

#include <nlohmann/json.hpp>

/**
 * @brief Interface for components that can be serialized to JSON.
 */
class ISavable
{
public:
    virtual ~ISavable() = default;

    /**
     * @brief Serializes this component to JSON.
     * @return JSON object including a "type" field, used by
     * ComponentRegistry to reconstruct the component on load.
     */
    virtual nlohmann::json toJson() const = 0;
};
