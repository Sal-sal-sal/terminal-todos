#include "UI/goals/events.hpp"

#include "backend/controllers/app_state.hpp"
#include "backend/goals/controller.hpp"

#include <algorithm>
#include <stdexcept>

namespace term_todos {
using ftxui::Event;
using Kind = AppState::ModalKind;

bool handle_goal_event(AppState& state, const Event& event, std::string& buffer) {
    const auto* node = find_goal(state.goals, state.goals.selected_id);
    if (event == Event::Character('j') || event == Event::ArrowDown) navigate_goal(state, 1);
    else if (event == Event::Character('k') || event == Event::ArrowUp) navigate_goal(state, -1);
    else if (event == Event::Character('n')) { buffer.clear(); state.modal = Kind::AddGoal; }
    else if (!node) return false;
    else if (event == Event::Character(' ')) toggle_goal(state);
    else if (event == Event::Return) {
        if (!state.goals.collapsed.erase(node->id)) state.goals.collapsed.insert(node->id);
    } else if (event == Event::Character('h') || event == Event::ArrowLeft) {
        const bool children = std::any_of(state.goals.nodes.begin(), state.goals.nodes.end(),
            [&](const GoalNode& child) { return child.parent_id == node->id; });
        if (children && !state.goals.collapsed.count(node->id)) state.goals.collapsed.insert(node->id);
        else if (node->parent_id) state.goals.selected_id = node->parent_id;
    } else if (event == Event::Character('l') || event == Event::ArrowRight) {
        state.goals.collapsed.erase(node->id);
    } else if (event == Event::Character('d')) {
        buffer.clear(); request_goal_deletion(state);
    } else {
        state.edit_target_id = node->id;
        buffer.clear();
        if (event == Event::Character('a') || event == Event::Character('s')) state.modal = Kind::AddGoalChild;
        else if (event == Event::Character('e')) { state.modal = Kind::EditGoal; buffer = node->title; }
        else if (event == Event::Character('D')) { state.modal = Kind::DescribeGoal; buffer = node->description; }
        else if (event == Event::Character('H')) {
            state.goals.habit_cursor = 0;
            state.modal = Kind::LinkGoalHabit;
        }
        else if (event == Event::Character('N')) state.modal = Kind::AddGoalHabit;
        else return false;
    }
    return true;
}

bool commit_goal_modal(AppState& state, const std::string& buffer) {
    switch (state.modal) {
        case Kind::AddGoal: add_goal(state, buffer, 0); break;
        case Kind::AddGoalChild: add_goal(state, buffer, state.edit_target_id); break;
        case Kind::EditGoal: edit_goal(state, buffer, false); break;
        case Kind::DescribeGoal: edit_goal(state, buffer, true); break;
        case Kind::AddGoalHabit: add_goal_habit(state, buffer, state.edit_target_id); break;
        case Kind::LinkGoalHabit: {
            std::string id = buffer;
            if (id.empty() && !state.habits.empty())
                id = std::to_string(state.habits.at(state.goals.habit_cursor).id);
            const auto habit = std::find_if(state.habits.begin(), state.habits.end(), [&](const Habit& item) {
                return std::to_string(item.id) == id;
            });
            if (habit == state.habits.end()) {
                state.set_error("Enter an existing habit ID from the list");
                return true;
            }
            add_goal(state, habit->name, state.edit_target_id, habit->id);
            break;
        }
        case Kind::DeleteGoal:
        case Kind::ConfirmDeleteGoal: confirm_goal_deletion(state, buffer); return true;
        default: return false;
    }
    state.modal = Kind::None;
    return true;
}

bool handle_goal_picker_event(AppState& state, const Event& event) {
    if (state.modal != Kind::LinkGoalHabit || state.habits.empty()) return false;
    int delta = 0;
    if (event == Event::ArrowDown) delta = 1;
    else if (event == Event::ArrowUp) delta = -1;
    else if (event == Event::PageDown) delta = 6;
    else if (event == Event::PageUp) delta = -6;
    else return false;
    state.goals.habit_cursor = std::clamp(state.goals.habit_cursor + delta, 0,
        static_cast<int>(state.habits.size()) - 1);
    return true;
}

} // namespace term_todos
