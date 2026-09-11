#include "backend/controllers/app_state.hpp"

#include "backend/controllers/state_helpers.hpp"

#include <algorithm>

namespace term_todos {

const TodoCollection* AppState::selected_collection() const {
    const auto it = std::find_if(collections.begin(), collections.end(),
        [this](const TodoCollection& item) {
            return item.id == selected_collection_id;
        });
    return it == collections.end() ? nullptr : &*it;
}

int AppState::active_collection_id() const {
    const auto* selected = selected_collection();
    if (selected) return selected->id;
    return collections.empty() ? 0 : collections.front().id;
}

std::vector<const TodoCollection*> AppState::visible_collections() const {
    std::vector<const TodoCollection*> result;
    for (const auto& collection : collections) {
        if (state_helpers::matches(collection.name, collection_query)) {
            result.push_back(&collection);
        }
    }
    return result;
}

void AppState::navigate_collection(int delta) {
    const auto visible = visible_collections();
    if (visible.empty()) return;

    int index = 0;
    for (int i = 0; i < static_cast<int>(visible.size()); ++i) {
        if (visible[i]->id == selected_collection_id) index = i;
    }
    index = (index + delta) % static_cast<int>(visible.size());
    if (index < 0) index += static_cast<int>(visible.size());
    selected_collection_id = visible[index]->id;
}

void AppState::open_selected_collection() {
    if (!selected_collection()) return;
    task_view = TaskView::Board;
    collection_query.clear();
    search_query.clear();
    selected_column = 0;
    selected_card = 0;
    for (int column = 0; column < 3; ++column) {
        if (!column_tasks(column).empty()) {
            selected_column = column;
            break;
        }
    }
}

void AppState::back_to_collections() {
    task_view = TaskView::Collections;
    search_query.clear();
}

void AppState::add_collection(const std::string& name) {
    if (name.empty()) return;
    TodoCollection collection;
    collection.id = state_helpers::next_id(collections);
    collection.name = name;
    collections.push_back(std::move(collection));
    selected_collection_id = collections.back().id;
    collection_query.clear();
    dirty = true;
}

void AppState::rename_selected_collection(const std::string& name) {
    if (name.empty()) return;
    for (auto& collection : collections) {
        if (collection.id == selected_collection_id) {
            collection.name = name;
            collection_query.clear();
            dirty = true;
            return;
        }
    }
}

void AppState::delete_selected_collection() {
    if (collections.size() <= 1) {
        status_message = "At least one collection is required";
        return;
    }
    const auto it = std::find_if(collections.begin(), collections.end(),
        [this](const TodoCollection& item) {
            return item.id == selected_collection_id;
        });
    if (it == collections.end()) return;

    const auto index = static_cast<std::size_t>(it - collections.begin());
    const int deleted_id = it->id;
    tasks.erase(std::remove_if(tasks.begin(), tasks.end(),
        [deleted_id](const Task& task) {
            return task.collection_id == deleted_id;
        }), tasks.end());
    collections.erase(it);
    selected_collection_id = collections[std::min(index, collections.size() - 1)].id;
    collection_query.clear();
    task_view = TaskView::Collections;
    dirty = true;
    clamp_selection();
}

} // namespace term_todos
