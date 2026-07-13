#pragma once

#include <nlohmann/json.hpp>

class ISavable {
 public:
  virtual ~ISavable() = default;
  virtual nlohmann::json ToJson() const = 0;
};
