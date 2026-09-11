#pragma once

#include <map>
#include <string>

namespace term_todos {

// A recurring habit tracked daily. History keys are ISO dates "YYYY-MM-DD".
//
// The streak is derived from history rather than stored, so it can never fall
// out of sync with the completion records. See compute_streak() below.
struct Habit {
    int id = 0;
    std::string name;
    std::map<std::string, bool> history;

    // Returns true if the habit was completed on the given ISO date.
    bool done_on(const std::string& iso_date) const;
    // Sets completion for the given ISO date.
    void set_done(const std::string& iso_date, bool done);
};

// Consecutive days completed counting back from today (inclusive). If today is
// not done yet, the streak still counts up to yesterday so a habit done every
// previous day reads as a live streak until the day ends.
int compute_streak(const Habit& habit, const std::string& today_iso);

} // namespace term_todos
