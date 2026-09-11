#pragma once

#include <nlohmann/json_fwd.hpp>

namespace term_todos {

struct AppState;

bool decode_state_records(const nlohmann::json& root, AppState& state);
nlohmann::json encode_state_records(const AppState& state);

} // namespace term_todos
