#include "UI/goals/view.hpp"

#include "UI/theme.hpp"
#include "backend/controllers/app_state.hpp"
#include "backend/models/date.hpp"
#include "backend/models/habit_markup.hpp"

#include <algorithm>

namespace term_todos {
using namespace ftxui;

namespace {

const Habit* linked_habit(const AppState& state, const GoalNode& node) {
    for (const auto& habit : state.habits) if (habit.id == node.habit_id) return &habit;
    return nullptr;
}

Element goal_row(const AppState& state, const GoalRow& row, bool children) {
    const auto& node = *row.node;
    const auto* habit = linked_habit(state, node);
    const bool done = habit ? habit->done_on(iso_today()) : node.done;
    const bool root = node.parent_id == 0;
    std::string branch = " ";
    // Keep the title readable even in very deeply nested maps.
    const std::size_t start = row.continuation.size() > 8 ? row.continuation.size() - 8 : 0;
    if (start) branch += "... ";
    for (std::size_t i = start; i < row.continuation.size(); ++i)
        branch += row.continuation[i] ? "│  " : "   ";
    if (!root) branch += row.last ? "└─ " : "├─ ";
    const std::string marker = children ? (state.goals.collapsed.count(node.id) ? "▸ " : "▾ ") : "  ";
    auto title = paragraph(node.habit_id ? plain_habit_text(node.title, true) : node.title) | flex;
    if (done) title |= color(theme::good());
    else title |= color(root ? theme::accent() : theme::muted());
    if (root) title |= bold;
    auto element = hbox({text(branch) | color(theme::dim()), text(marker),
        text(done ? "[x] " : "[ ] ") | color(done ? theme::good() : theme::dim()),
        text(root ? "GOAL " : node.habit_id ? "↻ " : "" ) | color(theme::accent()), title});
    if (state.goals.selected_id == node.id) element = element | inverted | focus;
    return element;
}

Element details(const AppState& state) {
    const auto* node = find_goal(state.goals, state.goals.selected_id);
    if (!node) return text("");
    Elements body;
    body.push_back(paragraph(node->habit_id ? plain_habit_text(node->title, true) : node->title)
        | bold | color(theme::accent()));
    if (node->habit_id) {
        const auto* habit = linked_habit(state, *node);
        body.push_back(paragraph(habit ? "Daily habit: " + plain_habit_text(habit->name, true) + " | Streak: "
            + std::to_string(compute_streak(*habit, iso_today())) + " days"
            : "Linked habit no longer exists") | color(habit ? theme::good() : theme::warn()));
    }
    body.push_back(paragraph(node->description.empty() ? "D: add a description" : node->description)
        | color(theme::muted()));
    return vbox(body) | size(HEIGHT, LESS_THAN, 6);
}

} // namespace

Element goals_footer(const AppState& state) {
    Elements hints;
    if (!state.status_message.empty()) hints.push_back(paragraph(state.status_message)
        | color(state.is_error_status ? theme::warn() : theme::good()));
    hints.push_back(vbox({
        text("n goal   a/s child   N new habit   H link habit   Space done") | color(theme::dim()),
        text("j/k select   Enter fold   h/l parent/open   e name   D desc   d delete") | color(theme::dim()),
    }));
    return vbox(hints);
}

Element goals_help() {
    return window(text("Goals / keyboard guide") | bold | color(theme::accent()), vbox({
        text("4 / Tab     open Goals / next tab"),
        text("n           create a new root goal"),
        text("a / s       add a child task to the selected node"),
        text("N           create a daily habit under this node"),
        text("H           choose and link an existing habit"),
        text("j/k, Up/Down  select a node"),
        text("Enter       fold or unfold descendants"),
        text("h / l       collapse or go to parent / expand"),
        text("Space       complete task / toggle habit today"),
        text("e / D       rename / edit description"),
        text("d           delete branch after two confirmations"),
        text("q           save and quit"), separator(),
        text("Goal changes save automatically. Linked habits share history."),
        text("Press any key to close") | color(theme::dim()),
    })) | clear_under | center;
}

Element render_goals(const AppState& state) {
    Elements rows;
    std::unordered_set<int> parents;
    for (const auto& node : state.goals.nodes) parents.insert(node.parent_id);
    for (const auto& row : goal_rows(state.goals))
        rows.push_back(goal_row(state, row, parents.count(row.node->id) != 0));
    if (rows.empty()) return vbox({
        text(""), text("GOALS / Turn a direction into daily action") | bold | color(theme::accent()),
        text(""), paragraph("Press n to create a goal. Add branches with a and daily habits with N."),
        text(""), text(" [ ] GOAL Become healthier") | color(theme::accent()),
        text("     ├─ [ ] Improve fitness"), text("     │  └─ [ ] Plan a workout"),
        text("     └─ [ ] ↻ Run every morning"), filler(),
    });
    return vbox({
        text("GOALS / Your map of next steps") | bold | color(theme::accent()), separator(),
        vbox(rows) | vscroll_indicator | yframe | flex,
        separator(), details(state),
    });
}

} // namespace term_todos
