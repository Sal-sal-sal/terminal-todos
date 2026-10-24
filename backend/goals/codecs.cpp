#include "backend/goals/codecs.hpp"
#include "backend/goals/model.hpp"

#include <nlohmann/json.hpp>
#include <unordered_map>

namespace term_todos {

bool decode_goals(const nlohmann::json& root, GoalMap& map) {
    if (!root.contains("goals")) return true;
    if (!root["goals"].is_array()) return false;
    GoalMap next;
    std::unordered_map<int, int> parents;
    for (const auto& item : root["goals"]) {
        GoalNode node;
        node.id = item.at("id").get<int>();
        node.parent_id = item.value("parent_id", 0);
        node.title = item.at("title").get<std::string>();
        node.description = item.value("description", "");
        node.done = item.value("done", false);
        node.habit_id = item.value("habit_id", 0);
        if (node.id <= 0 || node.parent_id < 0 || node.habit_id < 0 || node.title.empty()
            || !parents.emplace(node.id, node.parent_id).second) return false;
        next.nodes.push_back(std::move(node));
    }
    // Resolve every chain once, rejecting cycles and orphaned branches.
    std::unordered_set<int> resolved{0};
    for (const auto& node : next.nodes) {
        std::unordered_set<int> chain;
        int id = node.id;
        while (!resolved.count(id)) {
            if (!chain.insert(id).second || !parents.count(id)) return false;
            id = parents.at(id);
        }
        resolved.insert(chain.begin(), chain.end());
    }
    if (!next.nodes.empty()) next.selected_id = next.nodes.front().id;
    map = std::move(next);
    return true;
}

void encode_goals(nlohmann::json& root, const GoalMap& map) {
    // Optional, additive field: legacy files retain their canonical shape.
    if (map.nodes.empty()) return;
    root["goals"] = nlohmann::json::array();
    for (const auto& node : map.nodes) {
        root["goals"].push_back({{"id", node.id}, {"parent_id", node.parent_id},
            {"title", node.title}, {"description", node.description},
            {"done", node.done}, {"habit_id", node.habit_id}});
    }
}

} // namespace term_todos
