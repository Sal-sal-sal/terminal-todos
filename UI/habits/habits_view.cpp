#include "UI/habits/habits_view.hpp"

#include "UI/habits/heatmap.hpp"
#include "UI/habits/styled_text.hpp"
#include "UI/theme.hpp"
#include "backend/models/date.hpp"
#include "backend/models/habit.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace term_todos {

using namespace ftxui;

namespace {

Element detail_body(const AppState& state) {
    const Habit& habit = state.habits[state.selected_habit];
    constexpr int width = 33; // Reserve a cell for the scrollbar at 80 columns.
    constexpr int all_lines = std::numeric_limits<int>::max();
    Elements rows = {
        render_habit_text(habit.name, width, all_lines, true, true),
        separator(),
    };
    if (state.show_habit_description) {
        rows.push_back(text("DESCRIPTION  ·  Enter: heatmap") | color(theme::dim()));
        rows.push_back(habit.description.empty()
            ? text("Press D to add a description.") | color(theme::muted())
            : render_habit_text(habit.description, width, all_lines, true, false));
    } else {
        rows.push_back(habit.description.empty()
            ? text("D add description") | color(theme::dim())
            : render_habit_text(habit.description, width, 1, false, false));
        rows.push_back(render_heatmap(habit, 12));
    }
    return vbox(rows);
}

Element habit_row(const AppState& state, const Habit& habit, int index) {
    const bool focused = index == state.selected_habit;

    const bool done_today = habit.done_on(iso_today());
    auto check = text(done_today ? "[x]" : "[ ]") | color(done_today ? theme::good() : theme::dim());

    auto name = render_habit_text(habit.name, 20, 1, focused, true);

    std::ostringstream streak;
    streak << compute_streak(habit, iso_today()) << " day streak";
    auto streak_el = text(streak.str()) | color(theme::warn());

    auto cursor = text(focused ? "▸ " : "  ") | color(focused ? theme::accent() : theme::dim());

    return hbox({
        cursor,
        check,
        text(" "),
        name | flex,
        text("  "),
        streak_el,
    });
}

} // namespace

void scroll_habit_detail(AppState& state, int direction, bool to_edge) {
    if (state.habits.empty()) return;
    auto body = detail_body(state);
    body->ComputeRequirement();
    const int last_line = std::max(0, body->requirement().min_y - 1);
    state.habit_detail_line = to_edge ? (direction > 0 ? last_line : 0)
        : std::clamp(state.habit_detail_line + direction * 5, 0, last_line);
}

Element render_habits(const AppState& state) {
    // Compact summary: habits done today and best streak across all habits.
    const std::string today = iso_today();
    int done_today = 0;
    int best_streak = 0;
    for (const auto& habit : state.habits) {
        if (habit.done_on(today)) ++done_today;
        best_streak = std::max(best_streak, compute_streak(habit, today));
    }
    std::ostringstream line;
    line << done_today << "/" << state.habits.size() << " done today"
         << "    best streak: " << best_streak << "d";
    auto summary = hbox({
        text(line.str()) | color(theme::dim()),
        filler(),
    }) | border;

    // Left: habit list.
    Elements rows;
    if (state.habits.empty()) {
        rows.push_back(text("No habits yet") | color(theme::dim()));
    } else {
        for (int i = 0; i < static_cast<int>(state.habits.size()); ++i) {
            rows.push_back(habit_row(state, state.habits[i], i));
        }
    }
    auto list_box = vbox(rows) | border | flex;

    // Right: heatmap for the focused habit.
    Element detail;
    if (state.habits.empty()) {
        detail = text("(nothing to show)") | color(theme::dim()) | center | border | flex;
    } else {
        detail = vbox({
            detail_body(state) | focusPosition(0, state.habit_detail_line)
                | vscroll_indicator | yframe | flex,
            text("PgUp/PgDn scroll · Home/End") | color(theme::dim()),
        }) | border | flex;
    }

    return vbox({
        summary,
        hbox({
            list_box | size(WIDTH, LESS_THAN, 44),
            detail | flex,
        }) | flex,
    });
}

} // namespace term_todos
