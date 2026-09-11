#pragma once

#include "backend/models/habit.hpp"

#include <string>
#include <vector>

namespace term_todos {

struct AppState;

// Loads collections, tasks, and habits from JSON. Returns false and
// leaves the state untouched on any parse or I/O error. When `rebase` is true,
// habit history is shifted so the newest recorded day lines up with today
// (used when seeding from the bundled mock so the heatmap looks live on first
// run). Pass false for real user data.
//
// Canonical schema version 2:
// {
//   "schema_version": 2,
//   "collections": [{ "id": 1, "name": "Inbox" }],
//   "tasks": [
//     { "id": 12, "collection_id": 1, "title": "Fix login",
//       "status": "todo"|"in_progress"|"done", "labels": ["bug","ui"],
//       "priority": "none"|"low"|"medium"|"high", "due": "2026-07-20" }
//   ],
//   "habits": [
//     { "id": 1, "name": "Drink water",
//       "history": { "2026-07-15": true, "2026-07-14": true } }
//   ]
// }. Legacy files without collections are loaded into an automatic Inbox.
bool load_state(const std::string& path, AppState& out, bool rebase);

// Serialises the full app state to `path` atomically (write to a sibling temp
// file then rename). Returns false on I/O error. Leaves `state.data_path`
// untouched.
bool save_state(const std::string& path, const AppState& state);

// Writes a wide-format CSV of habit completion: one row per habit, one column
// per day over the trailing `days` days ending today, cells are "1" or empty.
// Dates with no record read as not-done. Returns false on I/O error.
bool export_habits_csv(const std::string& path, const AppState& state, int days);

// Isolated, thread-safe CSV export taking an explicit habit record vector.
bool export_habits_csv_records(const std::string& path, const std::vector<Habit>& habits, int days);

} // namespace term_todos
