#pragma once

#include <nlohmann/json_fwd.hpp>

namespace term_todos {
struct GoalMap;
bool decode_goals(const nlohmann::json& root, GoalMap& map);
void encode_goals(nlohmann::json& root, const GoalMap& map);
} // namespace term_todos
