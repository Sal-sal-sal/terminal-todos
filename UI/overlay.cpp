#include "UI/overlay.hpp"

#include "UI/theme.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

using namespace ftxui;

const char* modal_prompt(AppState::ModalKind kind) {
    switch (kind) {
        case AppState::ModalKind::AddCollection: return "New collection";
        case AppState::ModalKind::RenameCollection: return "Rename collection";
        case AppState::ModalKind::DeleteCollection: return "Delete collection";
        case AppState::ModalKind::SearchCollections: return "Find collections";
        case AppState::ModalKind::AddTask:     return "New task";
        case AppState::ModalKind::EditTask:    return "Edit task title";
        case AppState::ModalKind::SetDue:      return "Due date (YYYY-MM-DD or N days, blank to clear)";
        case AppState::ModalKind::Search:      return "Search tasks (title or label, blank to clear)";
        case AppState::ModalKind::AddHabit:    return "New habit";
        case AppState::ModalKind::RenameHabit: return "Rename habit";
        case AppState::ModalKind::None:        return nullptr;
    }
    return nullptr;
}

Element render_modal(const AppState& state, const Element& input_box) {
    const char* prompt = modal_prompt(state.modal);
    if (!prompt) return text("");

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

    auto box = window(
        text(prompt) | bold | color(theme::accent()),
        vbox({
            input_box,
            text("Enter to confirm - Esc to cancel") | color(theme::dim()),
        })
    ) | border | color(theme::muted());

    return box | clear_under | center;
}

namespace {

Element help_row(const char* key, const char* desc) {
    return hbox({
        text(key) | color(theme::accent()) | size(WIDTH, EQUAL, 10),
        text(desc) | color(theme::muted()),
    });
}

} // namespace

Element render_help() {
    auto body = vbox({
        text("Keybindings") | bold | color(theme::accent()),
        separator(),
        text("Global") | bold | color(theme::dim()),
        help_row("1 / 2", "switch tab"),
        help_row("Tab",   "cycle tab"),
        help_row("?",     "this help"),
        help_row("x",     "export habits to CSV"),
        help_row("q",     "quit (saves on exit)"),
        separator(),
        text("Collections") | bold | color(theme::dim()),
        help_row("j / k", "select collection"),
        help_row("Enter/l", "open collection"),
        help_row("n/e/d", "new / rename / delete"),
        help_row("f",     "find collections"),
        separator(),
        text("Collection board") | bold | color(theme::dim()),
        help_row("h / l", "move card across columns"),
        help_row("j / k", "select card"),
        help_row("n",     "new task"),
        help_row("e",     "edit title"),
        help_row("d",     "delete"),
        help_row("[ / ]", "cycle priority"),
        help_row("u",     "set due date"),
        help_row("f",     "search / filter"),
        help_row("b / Esc", "back to collections"),
        separator(),
        text("Habits") | bold | color(theme::dim()),
        help_row("j / k", "select habit"),
        help_row("space", "toggle today"),
        help_row("n",     "new habit"),
        help_row("e",     "rename"),
        help_row("d",     "delete"),
        text(" "),
        text("Press ? or Esc to close") | color(theme::dim()),
    });
    return body | border | color(theme::muted()) | clear_under | center;
}

std::string export_path(const std::string& data_path) {
    if (data_path.empty()) return "term-todos-habits.csv";
    const auto pos = data_path.find_last_of('.');
    const std::string stem = (pos == std::string::npos) ? data_path : data_path.substr(0, pos);
    return stem + ".csv";
}

} // namespace term_todos
