#pragma once

#include "backend/controllers/app_state.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

// Renders the Kanban board: three columns (Todo / In Progress / Done) side by
// side, with the focused column and card highlighted.
ftxui::Element render_board(const AppState& state);

} // namespace term_todos
