#include "UI/events/habit_events.hpp"

namespace term_todos {

using namespace ftxui;

bool handle_habit_event(AppState& state, const Event& event, std::string& input) {
    if (event == Event::ArrowUp || event == Event::Character('k')) {
        state.navigate_habit(-1);
    } else if (event == Event::ArrowDown || event == Event::Character('j')) {
        state.navigate_habit(1);
    } else if (event == Event::Character(' ')) {
        state.toggle_habit_today();
    } else if (event == Event::Character('n')) {
        input.clear();
        state.modal = AppState::ModalKind::AddHabit;
    } else if (event == Event::Character('e')) {
        if (!state.habits.empty()) {
            input = state.habits[state.selected_habit].name;
            state.edit_target_id = state.habits[state.selected_habit].id;
            state.modal = AppState::ModalKind::RenameHabit;
        }
    } else if (event == Event::Character('d')) {
        state.delete_focused_habit();
    } else if (event == Event::Escape) {
    } else {
        return false;
    }
    return true;
}

} // namespace term_todos
