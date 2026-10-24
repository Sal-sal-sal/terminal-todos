#pragma once

#include <ftxui/dom/elements.hpp>

namespace term_todos {
struct AppState;
ftxui::Element render_goals(const AppState& state);
ftxui::Element goals_footer(const AppState& state);
ftxui::Element goals_help();
} // namespace term_todos
