#include "backend/controllers/app_state.hpp"

#include "backend/controllers/state_helpers.hpp"

namespace term_todos {
namespace {

Task* focused_task_mutable(AppState& state) {
    return const_cast<Task*>(state.focused_task());
}

} // namespace

void AppState::navigate_sub_todo(int delta) {
    const Task* task = focused_task();
    if (!task || task->sub_todos.empty()) {
        selected_sub_todo = 0;
        return;
    }
    const int count = static_cast<int>(task->sub_todos.size());
    selected_sub_todo = (selected_sub_todo + delta) % count;
    if (selected_sub_todo < 0) selected_sub_todo += count;
}

void AppState::add_sub_todo(const std::string& title) {
    Task* task = focused_task_mutable(*this);
    if (!task || title.empty()) return;
    task->sub_todos.push_back({title, false});
    selected_sub_todo = static_cast<int>(task->sub_todos.size()) - 1;
    dirty = true;
}

void AppState::toggle_selected_sub_todo() {
    Task* task = focused_task_mutable(*this);
    if (!task || task->sub_todos.empty()) return;
    selected_sub_todo = state_helpers::clamp_int(
        selected_sub_todo, 0, static_cast<int>(task->sub_todos.size()) - 1);
    auto& sub_todo = task->sub_todos[selected_sub_todo];
    sub_todo.done = !sub_todo.done;
    dirty = true;
}

void AppState::delete_selected_sub_todo() {
    Task* task = focused_task_mutable(*this);
    if (!task || task->sub_todos.empty()) return;
    selected_sub_todo = state_helpers::clamp_int(
        selected_sub_todo, 0, static_cast<int>(task->sub_todos.size()) - 1);
    task->sub_todos.erase(task->sub_todos.begin() + selected_sub_todo);
    selected_sub_todo = task->sub_todos.empty()
        ? 0
        : state_helpers::clamp_int(selected_sub_todo, 0,
                                   static_cast<int>(task->sub_todos.size()) - 1);
    dirty = true;
}

} // namespace term_todos
