#pragma once

#include <string>

namespace term_todos {
struct AppState;
void navigate_goal(AppState& state, int delta);
void add_goal(AppState& state, const std::string& title, int parent, int habit = 0);
void add_goal_habit(AppState& state, const std::string& name, int parent);
void edit_goal(AppState& state, const std::string& value, bool description);
void toggle_goal(AppState& state);
void save_goals(AppState& state);
void request_goal_deletion(AppState& state);
void confirm_goal_deletion(AppState& state, const std::string& value);
} // namespace term_todos
