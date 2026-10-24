#include "backend/goals/controller.hpp"

#include "backend/controllers/app_state.hpp"
#include "backend/models/date.hpp"
#include "backend/store/json_store.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace term_todos {

void save_goals(AppState& state) {
    state.dirty = true;
    if (state.read_only_data || state.data_path.empty()) return;
    if (!save_state(state.data_path, state)) {
        state.set_error("Could not save Goals. Changes kept in memory; q retries.");
        return;
    }
    state.dirty = false;
    state.set_status("Goals saved");
}

void navigate_goal(AppState& state, int delta) {
    const auto rows = goal_rows(state.goals);
    if (rows.empty()) { state.goals.selected_id = 0; return; }
    int index = 0;
    for (std::size_t i = 0; i < rows.size(); ++i)
        if (rows[i].node->id == state.goals.selected_id) index = static_cast<int>(i);
    index = std::clamp(index + delta, 0, static_cast<int>(rows.size()) - 1);
    state.goals.selected_id = rows[index].node->id;
}

void add_goal(AppState& state, const std::string& title, int parent, int habit) {
    if (title.find_first_not_of(" \t\r\n") == std::string::npos) return;
    if (parent && !find_goal(state.goals, parent)) throw std::runtime_error("Parent no longer exists");
    int id = 0;
    for (const auto& node : state.goals.nodes) id = std::max(id, node.id);
    if (id == std::numeric_limits<int>::max()) throw std::runtime_error("Goal IDs exhausted");
    state.goals.nodes.push_back({id + 1, parent, title, "", false, habit});
    state.goals.selected_id = id + 1;
    state.goals.collapsed.erase(parent);
    save_goals(state);
}

void edit_goal(AppState& state, const std::string& value, bool description) {
    for (auto& node : state.goals.nodes) {
        if (node.id != state.edit_target_id) continue;
        if (description) node.description = value;
        else if (value.find_first_not_of(" \t\r\n") != std::string::npos) node.title = value;
        else return;
        save_goals(state);
        return;
    }
}

void toggle_goal(AppState& state) {
    for (auto& node : state.goals.nodes) {
        if (node.id != state.goals.selected_id) continue;
        if (node.habit_id) {
            for (auto& habit : state.habits) {
                if (habit.id != node.habit_id) continue;
                const auto today = iso_today();
                habit.set_done(today, !habit.done_on(today));
                save_goals(state);
                return;
            }
            state.set_error("Linked habit was deleted. Remove this link and add another with H.");
            return;
        }
        node.done = !node.done;
        save_goals(state);
        return;
    }
}

} // namespace term_todos
