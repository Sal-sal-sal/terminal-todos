#include "UI/habits/habits_view.hpp"

#include "UI/habits/heatmap.hpp"
#include "UI/theme.hpp"
#include "backend/models/date.hpp"
#include "backend/models/habit.hpp"

#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <sstream>
#include <string>

namespace term_todos {

using namespace ftxui;

namespace {

Element habit_row(const AppState& state, const Habit& habit, int index) {
    const bool focused = index == state.selected_habit;

    const bool done_today = habit.done_on(iso_today());
    auto check = text(done_today ? "[x]" : "[ ]") | color(done_today ? theme::good() : theme::dim());

    auto name = text(habit.name);
    if (focused) {
        name |= bold;
        name |= color(theme::accent());
    } else {
        name |= color(theme::muted());
    }

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
        const Habit& focused = state.habits[state.selected_habit];
        detail = vbox({
            text(focused.name) | bold | color(theme::accent()),
            separator(),
            render_heatmap(focused, 12),
            filler(),
        }) | border | flex;
    }

    return vbox({
        summary,
        hbox({
            list_box | size(WIDTH, GREATER_THAN, 34),
            detail | flex,
        }) | flex,
    });
}

} // namespace term_todos
