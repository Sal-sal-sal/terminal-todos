#include "UI/event_handler.hpp"

#include "UI/events/habit_events.hpp"
#include "UI/events/task_events.hpp"
#include "UI/overlay.hpp"
#include "backend/store/json_store.hpp"

namespace term_todos {

using namespace ftxui;

bool handle_event(AppState& state, const Event& event, ScreenInteractive& screen,
                  std::string& input_buffer) {
    if (!state.status_message.empty()) state.status_message.clear();

    if (event == Event::Character('q')) {
        screen.Exit();
        return true;
    }
    if (event == Event::Character('?')) {
        state.show_help = !state.show_help;
        return true;
    }
    if (event == Event::Character('x')) {
        const std::string path = export_path(state.data_path);
        state.status_message = export_habits_csv(path, state, 90)
            ? "Exported 90 days of habits to " + path
            : "Export failed: could not write " + path;
        return true;
    }
    if (event == Event::Character('1')) {
        state.set_tab(AppState::Tab::Habits);
        return true;
    }
    if (event == Event::Character('2')) {
        state.set_tab(AppState::Tab::Tasks);
        return true;
    }
    if (event == Event::Tab) {
        state.set_tab(state.tab == AppState::Tab::Tasks
                          ? AppState::Tab::Habits
                          : AppState::Tab::Tasks);
        return true;
    }
    if (state.tab == AppState::Tab::Tasks) {
        return handle_task_event(state, event, input_buffer);
    }
    return handle_habit_event(state, event, input_buffer);
}

void commit_modal(AppState& state, const std::string& buffer) {
    switch (state.modal) {
        case AppState::ModalKind::AddCollection:
            state.add_collection(buffer);
            break;
        case AppState::ModalKind::RenameCollection:
            state.rename_selected_collection(buffer);
            break;
        case AppState::ModalKind::DeleteCollection:
            state.delete_selected_collection();
            break;
        case AppState::ModalKind::SearchCollections: {
            state.collection_query = buffer;
            const auto visible = state.visible_collections();
            if (!visible.empty()) state.selected_collection_id = visible.front()->id;
            break;
        }
        case AppState::ModalKind::AddHabit:
            state.add_habit(buffer);
            break;
        case AppState::ModalKind::RenameHabit:
            state.rename_focused_habit(buffer);
            break;
        case AppState::ModalKind::AddTask:
            state.add_task(buffer);
            break;
        case AppState::ModalKind::EditTask:
            state.rename_focused_task(buffer);
            break;
        case AppState::ModalKind::SetDue:
            state.set_focused_due(buffer);
            break;
        case AppState::ModalKind::Search:
            state.search_query = buffer;
            state.clamp_selection();
            break;
        case AppState::ModalKind::None:
            break;
    }
    state.modal = AppState::ModalKind::None;
}

} // namespace term_todos
