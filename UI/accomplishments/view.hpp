#pragma once
#include "backend/controllers/app_state.hpp"
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

namespace term_todos {
ftxui::Element render_accomplishments(const AppState& state);
bool handle_accomplishment_event(AppState& state, const ftxui::Event& event, std::string& input);
}
