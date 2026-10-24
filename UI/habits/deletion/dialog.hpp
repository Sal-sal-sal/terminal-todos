#pragma once

#include "backend/controllers/app_state.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

ftxui::Element render_habit_deletion(const AppState& state,
                                    const ftxui::Element& input_box);

} // namespace term_todos
