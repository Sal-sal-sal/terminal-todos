#include "backend/store/json_codecs.hpp"

#include "backend/controllers/app_state.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <set>
#include <string>

namespace term_todos {

namespace {

TaskStatus status_from_string(const std::string& value) {
    if (value == "in_progress") return TaskStatus::InProgress;
    if (value == "done") return TaskStatus::Done;
    return TaskStatus::Todo;
}

const char* status_to_string(TaskStatus status) {
    switch (status) {
        case TaskStatus::Todo: return "todo";
        case TaskStatus::InProgress: return "in_progress";
        case TaskStatus::Done: return "done";
    }
    return "todo";
}

void decode_collections(const nlohmann::json& root, AppState& state) {
    std::set<int> ids;
    if (root.contains("collections") && root["collections"].is_array()) {
        for (const auto& item : root["collections"]) {
            const int id = item.value("id", 0);
            const std::string name = item.value("name", "");
            if (id > 0 && !name.empty() && ids.insert(id).second) {
                state.collections.push_back({id, name});
            }
        }
    }
    if (state.collections.empty()) state.collections.push_back({1, "Inbox"});
    state.selected_collection_id = state.collections.front().id;
}

bool known_collection(const AppState& state, int id) {
    return std::any_of(state.collections.begin(), state.collections.end(),
        [id](const TodoCollection& collection) {
            return collection.id == id;
        });
}

void decode_tasks(const nlohmann::json& root, AppState& state) {
    if (!root.contains("tasks") || !root["tasks"].is_array()) return;
    const int fallback_id = state.collections.front().id;
    for (const auto& item : root["tasks"]) {
        Task task;
        task.id = item.value("id", 0);
        task.title = item.value("title", "");
        task.status = status_from_string(item.value("status", "todo"));
        task.priority = priority_from_string(item.value("priority", "none"));
        task.due_date = item.value("due", "");
        task.collection_id = item.value("collection_id", fallback_id);
        if (!known_collection(state, task.collection_id)) task.collection_id = fallback_id;
        if (item.contains("labels") && item["labels"].is_array()) {
            for (const auto& label : item["labels"]) {
                if (label.is_string()) task.labels.push_back(label.get<std::string>());
            }
        }
        state.tasks.push_back(std::move(task));
    }
}

void decode_habits(const nlohmann::json& root, AppState& state) {
    if (!root.contains("habits") || !root["habits"].is_array()) return;
    for (const auto& item : root["habits"]) {
        Habit habit;
        habit.id = item.value("id", 0);
        habit.name = item.value("name", "");
        if (item.contains("history") && item["history"].is_object()) {
            for (auto it = item["history"].begin(); it != item["history"].end(); ++it) {
                if (it.value().is_boolean()) {
                    habit.history[it.key()] = it.value().get<bool>();
                }
            }
        }
        state.habits.push_back(std::move(habit));
    }
}

nlohmann::json encode_task(const Task& task) {
    nlohmann::json item = {
        {"id", task.id},
        {"collection_id", task.collection_id},
        {"title", task.title},
        {"status", status_to_string(task.status)},
        {"labels", task.labels},
        {"priority", priority_to_string(task.priority)},
    };
    if (!task.due_date.empty()) item["due"] = task.due_date;
    return item;
}

nlohmann::json encode_habit(const Habit& habit) {
    nlohmann::json history = nlohmann::json::object();
    for (const auto& [date, done] : habit.history) {
        if (done) history[date] = true;
    }
    return {{"id", habit.id}, {"name", habit.name}, {"history", history}};
}

} // namespace

bool decode_state_records(const nlohmann::json& root, AppState& state) {
    try {
        decode_collections(root, state);
        decode_tasks(root, state);
        decode_habits(root, state);
        return true;
    } catch (const nlohmann::json::exception&) {
        return false;
    }
}

nlohmann::json encode_state_records(const AppState& state) {
    nlohmann::json root = {
        {"schema_version", 2},
        {"collections", nlohmann::json::array()},
        {"tasks", nlohmann::json::array()},
        {"habits", nlohmann::json::array()},
    };
    for (const auto& collection : state.collections) {
        root["collections"].push_back({{"id", collection.id}, {"name", collection.name}});
    }
    for (const auto& task : state.tasks) root["tasks"].push_back(encode_task(task));
    for (const auto& habit : state.habits) root["habits"].push_back(encode_habit(habit));
    return root;
}

} // namespace term_todos
