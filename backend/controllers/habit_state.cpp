#include "backend/controllers/app_state.hpp"

#include "backend/controllers/state_helpers.hpp"
#include "backend/models/date.hpp"

namespace term_todos {

void AppState::navigate_habit(int delta) {
    const int count = static_cast<int>(habits.size());
    if (count == 0) {
        selected_habit = 0;
        return;
    }
    selected_habit = (selected_habit + delta) % count;
    if (selected_habit < 0) selected_habit += count;
}

void AppState::toggle_habit_today() {
    if (habits.empty()) return;
    Habit& habit = habits[selected_habit];
    const std::string today = iso_today();
    habit.set_done(today, !habit.done_on(today));
    dirty = true;
}

void AppState::add_habit(const std::string& name) {
    if (name.empty()) return;
    Habit habit;
    habit.id = state_helpers::next_id(habits);
    habit.name = name;
    habits.push_back(std::move(habit));
    selected_habit = static_cast<int>(habits.size()) - 1;
    dirty = true;
}

void AppState::delete_focused_habit() {
    if (habits.empty()) return;
    habits.erase(habits.begin() + selected_habit);
    selected_habit = state_helpers::clamp_int(
        selected_habit, 0, static_cast<int>(habits.size()) - 1);
    dirty = true;
}

void AppState::rename_focused_habit(const std::string& name) {
    if (habits.empty() || name.empty()) return;
    habits[selected_habit].name = name;
    dirty = true;
}

} // namespace term_todos
