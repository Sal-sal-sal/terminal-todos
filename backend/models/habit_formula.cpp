#include "backend/models/habit_formula.hpp"

namespace term_todos {

namespace {

bool consume_marked_part(const std::string& text, std::size_t& cursor,
                         std::string& before, std::string& part) {
    const std::size_t open = text.find('[', cursor);
    if (open == std::string::npos) return false;
    const std::size_t end = text.find(']', open + 1);
    if (end == std::string::npos || end == open + 1) return false;
    const std::size_t nested = text.find('[', open + 1);
    if (nested != std::string::npos && nested < end) return false;
    before = text.substr(cursor, open - cursor);
    part = text.substr(open + 1, end - open - 1);
    cursor = end + 1;
    return true;
}

} // namespace

std::optional<HabitFormula> parse_habit_formula(const std::string& name) {
    std::size_t cursor = 0;
    HabitFormula formula;
    if (!consume_marked_part(name, cursor, formula.prefix, formula.behavior)) {
        return std::nullopt;
    }
    formula.parts = 1;
    if (consume_marked_part(name, cursor, formula.before_place, formula.place)) {
        formula.parts = 2;
        if (consume_marked_part(name, cursor, formula.before_time, formula.time)) {
            formula.parts = 3;
        }
    }
    formula.suffix = name.substr(cursor);
    return formula;
}

} // namespace term_todos
