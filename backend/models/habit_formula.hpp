#pragma once

#include <optional>
#include <string>

namespace term_todos {

struct HabitFormula {
    int parts = 0;
    std::string prefix;
    std::string behavior;
    std::string before_place;
    std::string place;
    std::string before_time;
    std::string time;
    std::string suffix;
};

// Colors the first three nonempty [...] groups in any habit name by position.
// Text without a valid group remains ordinary text.
std::optional<HabitFormula> parse_habit_formula(const std::string& name);

} // namespace term_todos
