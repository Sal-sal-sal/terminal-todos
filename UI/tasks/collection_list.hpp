#pragma once

#include "backend/controllers/app_state.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

ftxui::Element render_collection_list(const AppState& state);

} // namespace term_todos
