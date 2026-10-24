#include "backend/goals/model.hpp"

#include <unordered_map>

namespace term_todos {

const GoalNode* find_goal(const GoalMap& map, int id) {
    for (const auto& node : map.nodes) if (node.id == id) return &node;
    return nullptr;
}

std::vector<GoalRow> goal_rows(const GoalMap& map) {
    std::unordered_map<int, std::vector<const GoalNode*>> children;
    for (const auto& node : map.nodes) children[node.parent_id].push_back(&node);
    std::vector<GoalRow> pending;
    auto append = [&](int parent, const std::vector<bool>& continuation) {
        const auto& siblings = children[parent];
        for (std::size_t i = siblings.size(); i > 0; --i)
            pending.push_back({siblings[i - 1], continuation, i == siblings.size()});
    };
    append(0, {});
    std::vector<GoalRow> rows;
    std::unordered_set<int> visited;
    while (!pending.empty()) {
        auto row = std::move(pending.back());
        pending.pop_back();
        if (!visited.insert(row.node->id).second) continue;
        rows.push_back(row);
        if (map.collapsed.count(row.node->id)) continue;
        auto continuation = row.continuation;
        if (row.node->parent_id != 0) continuation.push_back(!row.last);
        append(row.node->id, continuation);
    }
    return rows;
}

std::unordered_set<int> goal_subtree(const GoalMap& map, int id) {
    std::unordered_map<int, std::vector<int>> children;
    for (const auto& node : map.nodes) children[node.parent_id].push_back(node.id);
    std::unordered_set<int> result;
    std::vector<int> pending{id};
    while (!pending.empty()) {
        const int current = pending.back();
        pending.pop_back();
        if (!result.insert(current).second) continue;
        for (int child : children[current]) pending.push_back(child);
    }
    return result;
}

} // namespace term_todos
