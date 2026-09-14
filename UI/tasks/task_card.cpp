#include "UI/tasks/task_card.hpp"

#include "UI/theme.hpp"
#include "backend/models/date.hpp"
#include "backend/models/task.hpp"

#include <ftxui/dom/elements.hpp>

#include <sstream>

namespace term_todos {

using namespace ftxui;

namespace {

// Bracketed priority chip, e.g. "[H]". Returns an empty element when None so
// the layout stays flush for unprioritised tasks.
Element priority_chip(Priority priority) {
    const char* marker = priority_marker(priority);
    if (marker[0] == '\0') return text("");

    Color prio_color;
    switch (priority) {
        case Priority::High:   prio_color = Color::Red; break;
        case Priority::Medium: prio_color = theme::warn(); break;
        case Priority::Low:    prio_color = theme::todo(); break;
        default:               prio_color = theme::dim();
    }
    std::ostringstream label;
    label << "[" << marker << "]";
    return text(label.str()) | color(prio_color) | bold;
}

// Due-date chip in "due MM-DD" form, red when overdue, dim otherwise.
Element due_chip(const Task& task, const std::string& today) {
    if (task.due_date.empty()) return text("");

    // Show MM-DD compactly; the year is implied.
    std::string compact = task.due_date.size() >= 10
        ? task.due_date.substr(5) // "MM-DD"
        : task.due_date;

    const bool done = task.status == TaskStatus::Done;
    bool overdue = false;
    if (!done && !task.due_date.empty()) {
        try {
            overdue = serial_from_iso(task.due_date) < serial_from_iso(today);
        } catch (...) {
            overdue = false;
        }
    }

    std::ostringstream label;
    label << "due " << compact;
    return text(label.str()) | color(overdue ? Color::Red : theme::dim());
}

} // namespace

Element render_task_card(const Task& task, bool focused) {
    std::ostringstream id_str;
    id_str << "#" << task.id;

    // Selection cursor + task number + priority + title.
    auto cursor = text(focused ? "▸" : " ") | color(focused ? theme::accent() : theme::dim());
    auto id_el = text(id_str.str()) | color(theme::dim());
    auto title_el = text(task.title);
    if (focused) {
        title_el |= bold;
        title_el |= color(theme::accent());
    } else {
        title_el |= color(theme::muted());
    }

    Elements title_parts = { cursor, text(" "), id_el, text(" ") };
    Element prio = priority_chip(task.priority);
    if (!priority_marker(task.priority)[0]) {
        // Reserve a constant-width gap so titles align whether or not a
        // priority chip is present ("[H] " is four columns).
        title_parts.push_back(text("    "));
    } else {
        title_parts.push_back(prio);
        title_parts.push_back(text(" "));
    }
    title_parts.push_back(title_el);

    auto title_line = hbox({ hbox(title_parts), filler() });

    // Optional label chips.
    Elements chips;
    for (const auto& label : task.labels) {
        chips.push_back(text(label) | color(theme::label_color(label)));
        chips.push_back(text(" "));
    }
    Elements column = { title_line };
    if (!task.description.empty()) {
        column.push_back(text("    " + task.description) | color(theme::dim()));
    }
    if (!chips.empty()) {
        chips.pop_back(); // trailing gap
        Element due = due_chip(task, iso_today());
        Elements meta = { text("    "), hbox(chips) };
        if (!task.due_date.empty()) {
            meta.push_back(text("  "));
            meta.push_back(due);
        }
        column.push_back(hbox(meta));
    } else if (!task.due_date.empty()) {
        column.push_back(hbox({ text("    "), due_chip(task, iso_today()) }));
    }

    auto card = vbox(column);

    // Focused card gets a colored left rail so it stands out without relying on
    // background inversion (which hurts contrast against colored label text).
    if (focused) {
        return hbox({
            text("┃") | color(theme::accent()),
            card,
        });
    }
    return hbox({ text(" "), card });
}

} // namespace term_todos
