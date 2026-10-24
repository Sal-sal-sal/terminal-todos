#include "UI/events/habit_events.hpp"
#include "UI/habits/habits_view.hpp"

namespace term_todos {

using namespace ftxui;

bool handle_habit_event(AppState& state, const Event& event, std::string& input) {
    if (event == Event::ArrowUp || event == Event::Character('k')) {
        state.navigate_habit(-1);
    } else if (event == Event::ArrowDown || event == Event::Character('j')) {
        state.navigate_habit(1);
    } else if (event == Event::PageUp || event == Event::PageDown) {
        scroll_habit_detail(state, event == Event::PageUp ? -1 : 1);
    } else if (event == Event::Home || event == Event::End) {
        scroll_habit_detail(state, event == Event::Home ? -1 : 1, true);
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
    } else if (event == Event::Character('D')) {
        if (!state.habits.empty()) {
            input = state.habits[state.selected_habit].description;
            state.modal = AppState::ModalKind::EditHabitDescription;
            state.show_habit_description = true;
            state.habit_detail_line = 0;
        }
    } else if (event == Event::Return) {
        if (!state.habits.empty()) state.show_habit_description = !state.show_habit_description;
        state.habit_detail_line = 0;
    } else if (event == Event::Character('d')) {
        input.clear();
        state.request_habit_deletion();
    } else if (event == Event::Escape) {
        state.show_habit_description = false;
        state.habit_detail_line = 0;
    } else {
        return false;
    }
    return true;
}

} // namespace term_todos
