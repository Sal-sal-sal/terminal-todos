#include "backend/models/habit.hpp"
#include "backend/models/date.hpp"

#include <map>

namespace term_todos {

bool Habit::done_on(const std::string& iso_date) const {
    auto it = history.find(iso_date);
    return it != history.end() && it->second;
}

void Habit::set_done(const std::string& iso_date, bool done) {
    history[iso_date] = done;
    if (!done) {
        // Keep the map tidy: erase entries that are just "false".
        if (auto it = history.find(iso_date); it != history.end() && !it->second) {
            history.erase(it);
        }
    }
}

int compute_streak(const Habit& habit, const std::string& today_iso) {
    // Count consecutive completed days backwards. If today isn't done yet we
    // still credit yesterday backwards, so an unbroken run reads as live.
    std::string cursor = today_iso;
    if (!habit.done_on(cursor)) {
        cursor = iso_add_days(cursor, -1);
    }
    int streak = 0;
    while (habit.done_on(cursor)) {
        ++streak;
        cursor = iso_add_days(cursor, -1);
    }
    return streak;
}

} // namespace term_todos
