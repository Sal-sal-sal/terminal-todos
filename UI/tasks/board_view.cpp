#include "UI/tasks/board_view.hpp"

#include "UI/tasks/task_card.hpp"
#include "UI/theme.hpp"
#include "backend/models/task.hpp"

#include <ftxui/dom/elements.hpp>

#include <sstream>
#include <string>
#include <vector>

namespace term_todos {

using namespace ftxui;

namespace {

struct ColumnSpec {
    std::string title;
    Color color;
    TaskStatus status;
};

Element render_column(const AppState& state, const ColumnSpec& spec, int column_index) {
    const auto tasks = state.column_tasks(column_index);
    const bool focused = state.tab == AppState::Tab::Tasks &&
                         state.selected_column == column_index;

    // Header: title, count, colored when focused.
    auto header_left = hbox({
        text(spec.title) | bold | color(spec.color),
        text("  "),
        text(std::to_string(tasks.size())) | color(theme::dim()),
    });
    auto header = header_left | flex;

    Elements rows;
    rows.push_back(header);
    rows.push_back(separator());

    if (tasks.empty()) {
        rows.push_back(text("  empty") | color(theme::dim()) |
                       size(WIDTH, ftxui::GREATER_THAN, 18));
    } else {
        for (int i = 0; i < static_cast<int>(tasks.size()); ++i) {
            const bool card_focused = focused && i == state.selected_card;
            rows.push_back(render_task_card(*tasks[i], card_focused));
        }
    }

    auto body = vbox(rows);
    if (focused) {
        body |= borderDouble;
        body |= color(spec.color);
    } else {
        body |= border;
        body |= color(theme::dim());
    }
    return body;
}

} // namespace

// One-line summary above the board: totals and active filter.
Element stats_bar(const AppState& state);

Element render_board(const AppState& state) {
    const ColumnSpec columns[3] = {
        { "Todo",        theme::todo(),  TaskStatus::Todo },
        { "In Progress", theme::warn(),  TaskStatus::InProgress },
        { "Done",        theme::good(),  TaskStatus::Done },
    };

    Elements cols;
    for (int i = 0; i < 3; ++i) {
        cols.push_back(render_column(state, columns[i], i) | flex);
    }

    int total = 0;
    for (const auto& task : state.tasks) {
        if (task.collection_id == state.active_collection_id()) ++total;
    }
    Elements rows = {stats_bar(state), separator()};
    if (total == 0 && state.search_query.empty()) {
        rows.push_back(vbox({
            text("This collection has no tasks yet.") | bold,
            text(""),
            text("Press n to create a task.") | color(theme::dim()),
            text("Press b to return to collections.") | color(theme::dim()),
        }) | center | flex);
    } else {
        rows.push_back(hbox(cols) | flex);
    }
    return vbox(rows) | flex;
}

Element stats_bar(const AppState& state) {
    const int collection_id = state.active_collection_id();
    int total = 0;
    int done = 0;
    for (const auto& task : state.tasks) {
        if (task.collection_id != collection_id) continue;
        ++total;
        if (task.status == TaskStatus::Done) ++done;
    }
    const int pct = total == 0 ? 0 : (done * 100) / total;

    std::ostringstream summary;
    summary << total << " tasks  -  " << done << " done  (" << pct << "%)";

    Elements parts;
    const auto* collection = state.selected_collection();
    const std::string name = collection ? collection->name : "Unknown";
    parts.push_back(text(" COLLECTION: " + name + " ") | bold | color(theme::accent()));
    parts.push_back(text(summary.str()) | color(theme::dim()));

    if (!state.search_query.empty()) {
        int shown = 0;
        for (int c = 0; c < 3; ++c) shown += static_cast<int>(state.column_tasks(c).size());
        std::ostringstream f;
        f << "    filter: \"" << state.search_query << "\"  (" << shown << " shown)";
        parts.push_back(text(f.str()) | color(theme::accent()));
    }

    parts.push_back(filler());
    return hbox(parts);
}

} // namespace term_todos
