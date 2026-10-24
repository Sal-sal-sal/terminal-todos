#pragma once

#include <ftxui/dom/elements.hpp>

#include <string>

namespace term_todos {

ftxui::Element render_habit_text(const std::string& value, int width,
                                 int max_lines, bool focused,
                                 bool legacy_brackets);

} // namespace term_todos
