#pragma once

#include "backend/controllers/app_state.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace term_todos {

// Renders the whole application frame: tab bar, active view, and a footer with
// contextual key hints (or a transient status toast).
ftxui::Element render_app(const AppState& state);

// Builds the root component: a renderer over the state, plus input handling and
// modal/help overlays. The returned component drives screen.Loop(...).
ftxui::Component make_app(AppState& state, ftxui::ScreenInteractive& screen);

} // namespace term_todos
