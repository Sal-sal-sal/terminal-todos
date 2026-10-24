#include "UI/event_handler.hpp"
#include "UI/accomplishments/view.hpp"
#include "UI/goals/events.hpp"

#include "UI/events/habit_events.hpp"
#include "UI/events/task_events.hpp"
#include "UI/overlay.hpp"
#include "backend/core/async_runner.hpp"
#include "backend/store/json_store.hpp"

namespace term_todos {

using namespace ftxui;

bool handle_event(AppState& state, const Event& event, ScreenInteractive& screen,
                  std::string& input_buffer) {
    if (!state.status_message.empty() && !state.is_busy) state.clear_status();

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
        state.set_busy("Exporting habits to " + path + "...");
        auto habits = state.habits;
        default_async_runner().run_async(
            [path, habits = std::move(habits)]() {
                if (!export_habits_csv_records(path, habits, 90)) {
                    throw std::runtime_error("Could not write " + path);
                }
            },
            /*time_limit=*/std::chrono::milliseconds(5000),
            /*on_success=*/[&state, path]() {
                state.set_status("Exported 90 days of habits to " + path);
            },
            /*on_error=*/[&state](const std::string& err) {
                state.set_error("Export failed: " + err);
            },
            /*post_to_ui=*/[&screen](std::function<void()> fn) {
                screen.Post(std::move(fn));
            }
        );
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
    if (event == Event::Character('3')) {
        state.set_tab(AppState::Tab::Accomplishments);
        return true;
    }
    if (event == Event::Character('4')) {
        state.set_tab(AppState::Tab::Goals);
        return true;
    }
    if (event == Event::Tab) {
        state.set_tab(static_cast<AppState::Tab>((static_cast<int>(state.tab) + 1) % 4));
        return true;
    }
    if (state.tab == AppState::Tab::Goals) return handle_goal_event(state, event, input_buffer);
    if (state.tab == AppState::Tab::Accomplishments) return handle_accomplishment_event(state, event, input_buffer);
    if (state.tab == AppState::Tab::Tasks) {
        return handle_task_event(state, event, input_buffer);
    }
    return handle_habit_event(state, event, input_buffer);
}

void commit_modal(AppState& state, const std::string& buffer) {
    try {
        if (commit_goal_modal(state, buffer)) return;
        switch (state.modal) {
            case AppState::ModalKind::AddAccomplishment:
                state.add_accomplishment(buffer);
                break;
            case AppState::ModalKind::RenameAccomplishment:
                state.rename_accomplishment(buffer);
                break;
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
            case AppState::ModalKind::DeleteHabit:
            case AppState::ModalKind::ConfirmDeleteHabit:
                state.confirm_habit_deletion(buffer);
                return;
            case AppState::ModalKind::RenameHabit:
                state.rename_focused_habit(buffer);
                break;
            case AppState::ModalKind::EditHabitDescription:
                state.set_habit_description(buffer);
                break;
            case AppState::ModalKind::AddTask:
                state.add_task(buffer);
                break;
            case AppState::ModalKind::EditTask:
                state.rename_focused_task(buffer);
                break;
            case AppState::ModalKind::EditDescription:
                state.set_focused_description(buffer);
                break;
            case AppState::ModalKind::AddSubTodo:
                state.add_sub_todo(buffer);
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
            default: break;
        }
    } catch (const std::exception& ex) {
        state.set_error(std::string("Action failed: ") + ex.what());
    } catch (...) {
        state.set_error("Action failed due to an unknown error");
    }
    state.modal = AppState::ModalKind::None;
}

} // namespace term_todos
