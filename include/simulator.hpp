#pragma once
#include <string>
#include <nlohmann/json.hpp>

class Simulator {
public:
    nlohmann::json run(const nlohmann::json& params);
};
