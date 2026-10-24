#pragma once

#include "backend/controllers/app_state.hpp"
#include <ftxui/dom/elements.hpp>

namespace term_todos {
bool is_goal_modal(AppState::ModalKind kind);
ftxui::Element render_goal_modal(const AppState& state, const ftxui::Element& input);
} // namespace term_todos
