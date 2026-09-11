#include "UI/habits/heatmap.hpp"

#include "UI/theme.hpp"
#include "backend/models/date.hpp"

#include <ftxui/dom/elements.hpp>

#include <string>
#include <vector>

namespace term_todos {

using namespace ftxui;

namespace {

// Picks one of five intensity levels for a day, or -1 if there is no record.
int intensity(const Habit& habit, const std::string& iso) {
    auto it = habit.history.find(iso);
    if (it == habit.history.end()) return -1; // no data -> neutral empty
    return it->second ? 4 : 0;               // done -> max green, missed -> empty slot
}

// Builds the grid body: columns of weeks, each a vbox of 7 day-cells aligned to
// weekdays. Days outside the window are left blank so columns line up.
Element grid(const Habit& habit, const std::string& today_iso, int weeks) {
    // Align the start so the grid begins on a Monday.
    const int today_serial = serial_from_iso(today_iso);
    const int today_wd = weekday_monday_based(today_iso); // 0=Mon..6=Sun
    const int last_sunday_serial = today_serial + (6 - today_wd);
    const int start_serial = last_sunday_serial - (weeks - 1) * 7;

    Elements week_columns;
    for (int w = 0; w < weeks; ++w) {
        Elements day_cells;
        for (int d = 0; d < 7; ++d) {
            const int serial = start_serial + w * 7 + d;
            const std::string iso = iso_from_serial(serial);
            const int level = intensity(habit, iso);

            Element cell;
            if (level < 0) {
                // Future day or no data yet.
                if (serial > today_serial) {
                    cell = text(" ") | color(theme::dim());
                } else {
                    cell = text(" ") | color(theme::dim());
                }
            } else {
                cell = text(" ");
                cell |= bgcolor(theme::heatmap_level(level));
            }
            cell |= size(WIDTH, EQUAL, 2);
            cell |= size(HEIGHT, EQUAL, 1);
            day_cells.push_back(cell);
        }
        week_columns.push_back(vbox(day_cells));
    }
    return hbox(week_columns);
}

// Weekday labels Mon..Sun, aligned to the grid rows.
Element weekday_labels() {
    const char* labels[7] = { "Mon", "", "Wed", "", "Fri", "", "Sun" };
    Elements rows;
    for (int d = 0; d < 7; ++d) {
        rows.push_back(text(labels[d]) | color(theme::dim()) |
                       size(HEIGHT, EQUAL, 1));
    }
    return vbox(rows);
}

} // namespace

Element render_heatmap(const Habit& habit, int weeks) {
    const std::string today = iso_today();
    auto body = hbox({ weekday_labels(), text(" "), grid(habit, today, weeks) });

    auto legend = hbox({
        text("Less") | color(theme::dim()),
        text(" "),
        text(" ") | bgcolor(theme::heatmap_level(0)),
        text(" ") | bgcolor(theme::heatmap_level(1)),
        text(" ") | bgcolor(theme::heatmap_level(2)),
        text(" ") | bgcolor(theme::heatmap_level(3)),
        text(" ") | bgcolor(theme::heatmap_level(4)),
        text(" "),
        text("More") | color(theme::dim()),
    });

    return vbox({
        body,
        text(" "),
        hbox({ legend, filler() }),
    });
}

} // namespace term_todos
