#pragma once

#include "backend/controllers/app_state.hpp"

#include <ftxui/dom/elements.hpp>

#include <string>

namespace term_todos {

// Prompt text for the active modal dialog, or nullptr when none is open.
const char* modal_prompt(AppState::ModalKind kind);

// Renders the active modal dialog wrapping the given input-box element.
ftxui::Element render_modal(const AppState& state, const ftxui::Element& input_box);

// Renders the full keybindings help overlay.
ftxui::Element render_help();

// Builds a CSV export path next to the data file: same stem, ".csv" extension.
std::string export_path(const std::string& data_path);

} // namespace term_todos
