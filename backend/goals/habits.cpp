#include "backend/goals/controller.hpp"
#include "backend/controllers/app_state.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace term_todos {

void add_goal_habit(AppState& state, const std::string& name, int parent) {
    if (name.find_first_not_of(" \t\r\n") == std::string::npos) return;
    if (!find_goal(state.goals, parent)) throw std::runtime_error("Parent no longer exists");
    int id = 0;
    for (const auto& habit : state.habits) id = std::max(id, habit.id);
    if (id == std::numeric_limits<int>::max()) throw std::runtime_error("Habit IDs exhausted");
    Habit habit;
    habit.id = id + 1;
    habit.name = name;
    state.habits.push_back(std::move(habit));
    try {
        // Persist the habit and its goal link together, in one atomic write.
        add_goal(state, name, parent, id + 1);
    } catch (...) {
        state.habits.pop_back();
        throw;
    }
    state.selected_habit = static_cast<int>(state.habits.size()) - 1;
    state.habit_detail_line = 0;
}

} // namespace term_todos
