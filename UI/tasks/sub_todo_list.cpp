#include "UI/tasks/sub_todo_list.hpp"

#include "UI/theme.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

using namespace ftxui;

Element render_sub_todos(const Task& task, bool focused, int selected) {
    Elements rows;
    for (int index = 0; index < static_cast<int>(task.sub_todos.size()); ++index) {
        const auto& child = task.sub_todos[index];
        const bool active = focused && index == selected;
        auto cursor = text(active ? "› " : "  ") |
            color(active ? theme::accent() : theme::dim());
        auto check = text(child.done ? "[x] " : "[ ] ") |
            color(child.done ? theme::good() : theme::dim());
        auto title = text(child.title) |
            color(child.done ? theme::dim() : theme::muted());
        if (child.done) title |= dim;
        rows.push_back(hbox({text("    "), cursor, check, title}));
    }
    return vbox(rows);
}

} // namespace term_todos
