#pragma once

#include "backend/controllers/app_state.hpp"

#include <ftxui/component/event.hpp>

#include <string>

namespace term_todos {

bool handle_habit_event(AppState& state, const ftxui::Event& event,
                        std::string& input_buffer);

} // namespace term_todos
