#include "backend/controllers/app_state.hpp"

#include "backend/controllers/state_helpers.hpp"
#include "backend/models/date.hpp"
#include "backend/store/json_store.hpp"

namespace term_todos {

void AppState::navigate_habit(int delta) {
    habit_detail_line = 0;
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
    habit_detail_line = 0;
    dirty = true;
    if (data_path.empty() || read_only_data) return;
    if (!save_state(data_path, *this)) {
        set_error("Could not save habit to " + data_path + ". Changes kept in memory.");
        return;
    }
    dirty = false;
    set_status("Habit created and saved");
}

void AppState::delete_focused_habit() {
    if (habits.empty()) return;
    // Keep linked tasks, but never let a reused habit ID bind them to a new habit.
    for (auto& node : goals.nodes) {
        if (node.habit_id != habits[selected_habit].id) continue;
        node.habit_id = 0;
        node.done = habits[selected_habit].done_on(iso_today());
    }
    habit_detail_line = 0;
    habits.erase(habits.begin() + selected_habit);
    selected_habit = state_helpers::clamp_int(
        selected_habit, 0, static_cast<int>(habits.size()) - 1);
    if (habits.empty()) show_habit_description = false;
    dirty = true;
}

void AppState::rename_focused_habit(const std::string& name) {
    if (habits.empty() || name.empty()) return;
    habits[selected_habit].name = name;
    habit_detail_line = 0;
    dirty = true;
}

void AppState::set_habit_description(const std::string& description) {
    if (habits.empty()) return;
    Habit& habit = habits[selected_habit];
    if (habit.description == description) return;
    habit.description = description;
    habit_detail_line = 0;
    dirty = true;
}

} // namespace term_todos
