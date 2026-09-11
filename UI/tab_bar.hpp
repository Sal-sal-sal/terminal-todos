#pragma once

#include "backend/controllers/app_state.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

// Renders the top tab bar: [ Habits | Tasks ] with the active tab highlighted.
ftxui::Element render_tab_bar(const AppState& state);

} // namespace term_todos
