#pragma once

#include <string>
#include <vector>

namespace term_todos {

enum class HabitTone { Plain, Red, Green, Orange, Blue };

struct HabitSpan {
    std::string text;
    HabitTone tone = HabitTone::Plain;
};

// Explicit [#color]/[#end] tags work in names and descriptions.
// Legacy [behavior] [place] [time] coloring applies only to names without tags.
std::vector<HabitSpan> parse_habit_markup(const std::string& value,
                                         bool legacy_brackets);

std::string plain_habit_text(const std::string& value, bool legacy_brackets);

} // namespace term_todos
