#include "backend/controllers/app_state.hpp"

#include "backend/controllers/state_helpers.hpp"
#include "backend/models/date.hpp"

namespace term_todos {

std::vector<const Task*> AppState::column_tasks(int column) const {
    std::vector<const Task*> result;
    const int collection_id = active_collection_id();
    for (const auto& task : tasks) {
        if (task.collection_id == collection_id
            && Task::column_index(task.status) == column
            && state_helpers::matches_task(task, search_query)) {
            result.push_back(&task);
        }
    }
    return result;
}

const Task* AppState::focused_task() const {
    const auto visible = column_tasks(selected_column);
    if (visible.empty()) return nullptr;
    const int index = state_helpers::clamp_int(
        selected_card, 0, static_cast<int>(visible.size()) - 1);
    return visible[index];
}

void AppState::move_task_left() {
    if (selected_column <= 0) return;
    const Task* focused = focused_task();
    if (!focused) return;
    const int id = focused->id;
    for (auto& task : tasks) {
        if (task.id == id) {
            task.status = Task::column_to_status(--selected_column);
            dirty = true;
            clamp_selection();
            return;
        }
    }
}

void AppState::move_task_right() {
    if (selected_column >= 2) return;
    const Task* focused = focused_task();
    if (!focused) return;
    const int id = focused->id;
    for (auto& task : tasks) {
        if (task.id == id) {
            task.status = Task::column_to_status(++selected_column);
            dirty = true;
            clamp_selection();
            return;
        }
    }
}

void AppState::navigate_card(int delta) {
    const int count = static_cast<int>(column_tasks(selected_column).size());
    if (count == 0) {
        selected_card = 0;
        return;
    }
    selected_card = (selected_card + delta) % count;
    if (selected_card < 0) selected_card += count;
}

void AppState::navigate_column(int delta) {
    selected_column = state_helpers::clamp_int(selected_column + delta, 0, 2);
    selected_card = 0;
}

void AppState::add_task(const std::string& title) {
    if (title.empty()) return;
    if (collections.empty()) add_collection("Inbox");
    Task task;
    task.id = state_helpers::next_id(tasks);
    task.title = title;
    task.collection_id = active_collection_id();
    tasks.push_back(std::move(task));
    selected_column = 0;
    selected_card = static_cast<int>(column_tasks(0).size()) - 1;
    dirty = true;
}

void AppState::delete_focused_task() {
    const Task* focused = focused_task();
    if (!focused) return;
    const int id = focused->id;
    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
        if (it->id == id) {
            tasks.erase(it);
            dirty = true;
            clamp_selection();
            return;
        }
    }
}

void AppState::rename_focused_task(const std::string& title) {
    Task* focused = const_cast<Task*>(focused_task());
    if (!focused || title.empty()) return;
    focused->title = title;
    dirty = true;
}

void AppState::cycle_focused_priority() {
    Task* focused = const_cast<Task*>(focused_task());
    if (!focused) return;
    focused->priority = next_priority(focused->priority);
    dirty = true;
}

void AppState::set_focused_due(const std::string& spec) {
    Task* focused = const_cast<Task*>(focused_task());
    if (!focused) return;

    std::string value = spec;
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
        value.erase(value.begin());
    }
    while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
        value.pop_back();
    }
    if (value.empty()) {
        focused->due_date.clear();
        dirty = true;
        return;
    }
    try {
        if (value.find('-') != std::string::npos) {
            (void)serial_from_iso(value);
            focused->due_date = value;
        } else {
            focused->due_date = iso_add_days(iso_today(), std::stoi(value));
        }
        dirty = true;
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }
}

} // namespace term_todos
