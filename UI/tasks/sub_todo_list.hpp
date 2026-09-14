#pragma once

#include "backend/models/task.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

ftxui::Element render_sub_todos(const Task& task, bool focused, int selected);

} // namespace term_todos
