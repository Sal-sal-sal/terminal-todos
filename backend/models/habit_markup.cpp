#include "backend/models/habit_markup.hpp"

#include "backend/models/habit_formula.hpp"

namespace term_todos {

namespace {

void append(std::vector<HabitSpan>& spans, const std::string& value, HabitTone tone) {
    if (value.empty()) return;
    if (!spans.empty() && spans.back().tone == tone) spans.back().text += value;
    else spans.push_back({value, tone});
}

bool color_tag(const std::string& value, std::size_t at,
               HabitTone& tone, std::size_t& length) {
    struct Tag { const char* text; HabitTone tone; std::size_t length; };
    static constexpr Tag tags[] = {
        {"[#red]", HabitTone::Red, 6},
        {"[#green]", HabitTone::Green, 8},
        {"[#orange]", HabitTone::Orange, 9},
        {"[#blue]", HabitTone::Blue, 7},
        {"[#end]", HabitTone::Plain, 6},
    };
    for (const auto& tag : tags) {
        if (value.compare(at, tag.length, tag.text) != 0) continue;
        tone = tag.tone;
        length = tag.length;
        return true;
    }
    return false;
}

std::vector<HabitSpan> explicit_markup(const std::string& value, bool& found) {
    std::vector<HabitSpan> spans;
    HabitTone active = HabitTone::Plain;
    std::size_t start = 0;
    for (std::size_t at = 0; at < value.size();) {
        HabitTone next;
        std::size_t length = 0;
        if (value[at] != '[' || !color_tag(value, at, next, length)) {
            ++at;
            continue;
        }
        append(spans, value.substr(start, at - start), active);
        active = next;
        at += length;
        start = at;
        found = true;
    }
    append(spans, value.substr(start), active);
    return spans;
}

std::vector<HabitSpan> legacy_markup(const HabitFormula& formula) {
    std::vector<HabitSpan> spans;
    append(spans, formula.prefix, HabitTone::Plain);
    append(spans, formula.behavior, HabitTone::Red);
    append(spans, formula.before_place, HabitTone::Plain);
    append(spans, formula.place, HabitTone::Green);
    append(spans, formula.before_time, HabitTone::Plain);
    append(spans, formula.time, HabitTone::Blue);
    append(spans, formula.suffix, HabitTone::Plain);
    return spans;
}

} // namespace

std::vector<HabitSpan> parse_habit_markup(const std::string& value,
                                         bool legacy_brackets) {
    bool found = false;
    auto spans = explicit_markup(value, found);
    if (found || !legacy_brackets || value.find("[#") != std::string::npos) return spans;
    const auto formula = parse_habit_formula(value);
    return formula ? legacy_markup(*formula) : spans;
}

std::string plain_habit_text(const std::string& value, bool legacy_brackets) {
    std::string plain;
    for (const auto& span : parse_habit_markup(value, legacy_brackets)) plain += span.text;
    return plain;
}

} // namespace term_todos
