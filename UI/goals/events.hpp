#pragma once

#include <ftxui/component/event.hpp>
#include <string>

namespace term_todos {
struct AppState;
bool handle_goal_event(AppState& state, const ftxui::Event& event, std::string& buffer);
bool commit_goal_modal(AppState& state, const std::string& buffer);
bool handle_goal_picker_event(AppState& state, const ftxui::Event& event);
} // namespace term_todos
