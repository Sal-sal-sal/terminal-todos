#include "UI/overlay.hpp"
#include "UI/habits/deletion/dialog.hpp"
#include "UI/goals/modal.hpp"

#include "UI/theme.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

using namespace ftxui;

const char* modal_prompt(AppState::ModalKind kind) {
    switch (kind) {
        case AppState::ModalKind::AddAccomplishment: return "New accomplishment";
        case AppState::ModalKind::RenameAccomplishment: return "Rename accomplishment";
        case AppState::ModalKind::AddCollection: return "New collection";
        case AppState::ModalKind::RenameCollection: return "Rename collection";
        case AppState::ModalKind::DeleteCollection: return "Delete collection";
        case AppState::ModalKind::SearchCollections: return "Find collections";
        case AppState::ModalKind::AddTask:     return "New task";
        case AppState::ModalKind::EditTask:    return "Edit task title";
        case AppState::ModalKind::EditDescription: return "Edit task description (blank to clear)";
        case AppState::ModalKind::AddSubTodo: return "New sub-todo";
        case AppState::ModalKind::SetDue:      return "Due date (YYYY-MM-DD or N days, blank to clear)";
        case AppState::ModalKind::Search:      return "Search tasks (title or label, blank to clear)";
        case AppState::ModalKind::AddHabit:    return "New habit";
        case AppState::ModalKind::DeleteHabit: return "Delete habit";
        case AppState::ModalKind::ConfirmDeleteHabit: return "Final confirmation";
        case AppState::ModalKind::RenameHabit: return "Edit habit";
        case AppState::ModalKind::EditHabitDescription: return "Habit description (blank to clear)";
        case AppState::ModalKind::None:        return nullptr;
        default: return nullptr;
    }
    return nullptr;
}

Element render_modal(const AppState& state, const Element& input_box) {
    if (is_goal_modal(state.modal)) return render_goal_modal(state, input_box);
    const char* prompt = modal_prompt(state.modal);
    if (!prompt) return text("");

    if (state.modal == AppState::ModalKind::DeleteHabit
        || state.modal == AppState::ModalKind::ConfirmDeleteHabit) {
        return render_habit_deletion(state, input_box);
    }

    if (state.modal == AppState::ModalKind::DeleteCollection) {
        const auto* collection = state.selected_collection();
        int task_count = 0;
        if (collection) {
            for (const auto& task : state.tasks) {
                if (task.collection_id == collection->id) ++task_count;
            }
        }
        const std::string name = collection ? collection->name : "collection";
        auto body = vbox({
            text("Delete \"" + name + "\" and its "
                 + std::to_string(task_count) + " tasks?") | bold,
            text(""),
            text("Enter to delete - Esc to cancel") | color(theme::dim()),
        });
        return window(text(prompt) | bold | color(theme::warn()), body)
            | border | color(theme::warn()) | clear_under | center;
    }

    const bool habit_modal = state.modal == AppState::ModalKind::AddHabit
        || state.modal == AppState::ModalKind::RenameHabit;
    Elements fields = {input_box};
    if (habit_modal) {
        fields.push_back(hbox({
            text("i'm ") | color(theme::dim()),
            text("[running]") | color(Color::RedLight),
            text(" in ") | color(theme::dim()),
            text("[mountain]") | color(Color::GreenLight),
            text(" at ") | color(theme::dim()),
            text("[6:00]") | color(Color::BlueLight),
        }));
        fields.push_back(hbox({
            text("behavior") | color(Color::RedLight), text("  "),
            text("place") | color(Color::GreenLight), text("  "),
            text("time") | color(Color::BlueLight),
        }));
    }
    if (habit_modal || state.modal == AppState::ModalKind::EditHabitDescription) {
        fields.push_back(text("Colors: [#red] [#green] [#orange] [#blue] [#end]")
                         | color(theme::dim()));
    }
    fields.push_back(text("Enter to confirm - Esc to cancel") | color(theme::dim()));

    auto box = window(
        text(prompt) | bold | color(theme::accent()),
        vbox(fields)
    ) | border | color(theme::muted());

    return box | clear_under | center;
}

std::string export_path(const std::string& data_path) {
    if (data_path.empty()) return "term-todos-habits.csv";
    const auto pos = data_path.find_last_of('.');
    const std::string stem = (pos == std::string::npos) ? data_path : data_path.substr(0, pos);
    return stem + ".csv";
}

} // namespace term_todos
