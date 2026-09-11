#include "backend/controllers/app_state.hpp"

#include "backend/controllers/state_helpers.hpp"

#include <algorithm>

namespace term_todos {

void AppState::set_status(const std::string& message) {
    status_message = message;
    is_error_status = false;
    is_busy = false;
}

void AppState::set_error(const std::string& error_message) {
    status_message = error_message;
    is_error_status = true;
    is_busy = false;
}

void AppState::set_busy(const std::string& busy_message) {
    status_message = busy_message;
    is_busy = true;
    is_error_status = false;
}

void AppState::clear_status() {
    status_message.clear();
    is_error_status = false;
    is_busy = false;
}

void AppState::set_tab(Tab next) {
    tab = next;
    clamp_selection();
}

void AppState::clamp_selection() {
    if (collections.empty()) {
        selected_collection_id = 0;
        task_view = TaskView::Collections;
    } else {
        const auto selected = std::find_if(collections.begin(), collections.end(),
            [this](const TodoCollection& item) {
                return item.id == selected_collection_id;
            });
        if (selected == collections.end()) selected_collection_id = collections.front().id;
    }

    selected_column = state_helpers::clamp_int(selected_column, 0, 2);
    const int cards = static_cast<int>(column_tasks(selected_column).size());
    selected_card = cards == 0
        ? 0
        : state_helpers::clamp_int(selected_card, 0, cards - 1);
    selected_habit = habits.empty()
        ? 0
        : state_helpers::clamp_int(selected_habit, 0,
                                   static_cast<int>(habits.size()) - 1);
}

} // namespace term_todos
