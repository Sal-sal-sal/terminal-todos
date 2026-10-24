#pragma once

#include <string>
#include <unordered_set>
#include <vector>

namespace term_todos {

struct GoalNode {
    int id = 0;
    int parent_id = 0;
    std::string title;
    std::string description;
    bool done = false;
    int habit_id = 0;
};

struct GoalMap {
    std::vector<GoalNode> nodes;
    int selected_id = 0;
    std::unordered_set<int> collapsed;
    std::size_t deletion_count = 0;
    int habit_cursor = 0;
};

struct GoalRow {
    const GoalNode* node;
    std::vector<bool> continuation;
    bool last = true;
};

const GoalNode* find_goal(const GoalMap& map, int id);
std::vector<GoalRow> goal_rows(const GoalMap& map);
std::unordered_set<int> goal_subtree(const GoalMap& map, int id);

} // namespace term_todos
