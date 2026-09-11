#pragma once

#include "backend/controllers/app_state.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

// Renders the Habits tab: a list of habits with streaks on the left and the
// heatmap of the focused habit on the right.
ftxui::Element render_habits(const AppState& state);

} // namespace term_todos
