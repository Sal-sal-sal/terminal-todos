#pragma once

#include "backend/models/habit.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

// Renders a GitHub-style contribution grid for one habit: weeks as columns,
// weekdays (Mon..Sun) as rows, each cell colored by completion. The window ends
// at today and spans `weeks` weeks.
ftxui::Element render_heatmap(const Habit& habit, int weeks);

} // namespace term_todos
