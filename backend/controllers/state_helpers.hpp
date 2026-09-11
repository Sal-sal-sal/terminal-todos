#pragma once

#include "backend/models/task.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace term_todos::state_helpers {

inline int clamp_int(int value, int lo, int hi) {
    if (hi < lo) return lo;
    return std::max(lo, std::min(value, hi));
}

inline std::string lower(std::string value) {
    for (char& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

inline bool matches(const std::string& value, const std::string& query) {
    return query.empty() || lower(value).find(lower(query)) != std::string::npos;
}

inline bool matches_task(const Task& task, const std::string& query) {
    if (matches(task.title, query)) return true;
    for (const auto& label : task.labels) {
        if (matches(label, query)) return true;
    }
    return false;
}

template <typename Item>
int next_id(const std::vector<Item>& items) {
    int max_id = 0;
    for (const auto& item : items) max_id = std::max(max_id, item.id);
    return max_id + 1;
}

} // namespace term_todos::state_helpers
