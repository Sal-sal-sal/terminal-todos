#pragma once

#include "backend/controllers/app_state.hpp"

#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <string>

namespace term_todos {

// Handles a keyboard event against the state when no modal/help overlay is
// open. Returns true if the event was consumed. `input_buffer` is prefilled by
// the handlers that open a modal (e.g. edit/rename) so the dialog shows the
// current value. Quits by calling screen.Exit().
bool handle_event(AppState& state, const ftxui::Event& event,
                  ftxui::ScreenInteractive& screen, std::string& input_buffer);

// Applies the buffered text to the open modal, then closes it.
void commit_modal(AppState& state, const std::string& buffer);

} // namespace term_todos
