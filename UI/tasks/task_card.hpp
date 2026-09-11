#pragma once

#include "backend/models/task.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

// Renders a single Kanban card. `focused` controls the highlight and cursor
// marker so the board can indicate the selected card.
ftxui::Element render_task_card(const Task& task, bool focused);

} // namespace term_todos
