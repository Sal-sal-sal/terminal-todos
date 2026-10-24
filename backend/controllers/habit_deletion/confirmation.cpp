#include "backend/controllers/app_state.hpp"

#include <algorithm>

namespace term_todos {

const Habit* AppState::pending_habit_deletion() const {
    const auto target = std::find_if(habits.begin(), habits.end(), [this](const Habit& habit) {
        return habit.id == edit_target_id;
    });
    return target == habits.end() ? nullptr : &*target;
}

void AppState::request_habit_deletion() {
    if (habits.empty()) return;
    const Habit& habit = habits[selected_habit];
    edit_target_id = habit.id;
    habit_delete_history_count = habit.history.size();
    clear_status();
    modal = ModalKind::DeleteHabit;
}

void AppState::confirm_habit_deletion(const std::string& confirmation) {
    if (modal != ModalKind::DeleteHabit && modal != ModalKind::ConfirmDeleteHabit) return;
    const Habit* habit = pending_habit_deletion();
    if (!habit) {
        modal = ModalKind::None;
        set_error("Habit no longer exists; nothing was deleted.");
        return;
    }
    if (habit->history.size() != habit_delete_history_count) {
        habit_delete_history_count = habit->history.size();
        modal = ModalKind::DeleteHabit;
        set_error("History changed. Please review the deletion again.");
        return;
    }
    if (modal == ModalKind::DeleteHabit) {
        modal = ModalKind::ConfirmDeleteHabit;
        clear_status();
        return;
    }
    const std::string count = std::to_string(habit_delete_history_count);
    if (confirmation != count) {
        set_error("Type " + count + " to confirm deletion.");
        return;
    }
    selected_habit = static_cast<int>(habit - habits.data());
    delete_focused_habit();
    modal = ModalKind::None;
    edit_target_id = 0;
    set_status("Deleted 1 habit and " + count + " history record"
        + (habit_delete_history_count == 1 ? "." : "s."));
}

} // namespace term_todos
