#include "backend/goals/controller.hpp"
#include "backend/controllers/app_state.hpp"

#include <algorithm>

namespace term_todos {

void request_goal_deletion(AppState& state) {
    if (!find_goal(state.goals, state.goals.selected_id)) return;
    state.edit_target_id = state.goals.selected_id;
    state.goals.deletion_count = goal_subtree(state.goals, state.edit_target_id).size();
    state.modal = AppState::ModalKind::DeleteGoal;
}

void confirm_goal_deletion(AppState& state, const std::string& value) {
    const auto* node = find_goal(state.goals, state.edit_target_id);
    if (!node) { state.modal = AppState::ModalKind::None; return; }
    const auto ids = goal_subtree(state.goals, node->id);
    if (state.goals.deletion_count != ids.size()) {
        state.goals.deletion_count = ids.size();
        state.modal = AppState::ModalKind::DeleteGoal;
        return;
    }
    if (state.modal == AppState::ModalKind::DeleteGoal) {
        state.modal = AppState::ModalKind::ConfirmDeleteGoal;
        return;
    }
    if (state.modal != AppState::ModalKind::ConfirmDeleteGoal) return;
    if (value != std::to_string(ids.size())) {
        state.set_error("Type exactly " + std::to_string(ids.size()) + " to delete this branch");
        return;
    }
    const int parent = node->parent_id;
    auto& nodes = state.goals.nodes;
    nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&](const GoalNode& item) {
        return ids.count(item.id) != 0;
    }), nodes.end());
    for (int id : ids) state.goals.collapsed.erase(id);
    state.goals.selected_id = parent;
    navigate_goal(state, 0);
    state.modal = AppState::ModalKind::None;
    save_goals(state);
}

} // namespace term_todos
