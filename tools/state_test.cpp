// Deterministic integration test for the data layer: loads the bundled mock,
// applies a sequence of AppState mutations, saves, reloads, and asserts the
// mutations round-tripped through JSON. Exits non-zero on any mismatch.
//
// This complements the snapshot harness: it covers persistence + mutations
// without the TTY, while the interactive binary covers event wiring.

#include "backend/controllers/app_state.hpp"
#include "backend/models/date.hpp"
#include "backend/models/task.hpp"
#include "backend/store/json_store.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#ifndef TERM_TODOS_DATA_DIR
#define TERM_TODOS_DATA_DIR "."
#endif

namespace {

int failures = 0;

void check(bool ok, const std::string& what) {
    if (ok) {
        std::cout << "  ok   " << what << "\n";
    } else {
        std::cout << "  FAIL " << what << "\n";
        ++failures;
    }
}

} // namespace

int main() {
    using namespace term_todos;

    const std::string mock = std::string(TERM_TODOS_DATA_DIR) + "/mock.json";
    const std::filesystem::path tmp_dir = std::getenv("TMPDIR")
        ? std::getenv("TMPDIR") : "/tmp";
    const std::string tmp = (tmp_dir / "term-todos-state-test.json").string();
    std::remove(tmp.c_str());

    AppState s;
    check(load_state(mock, s, /*rebase=*/false), "load bundled mock");
    check(s.collections.size() == 1 && s.collections.front().name == "Inbox",
          "legacy data migrated to Inbox");
    bool legacy_linked = true;
    for (const auto& task : s.tasks) {
        legacy_linked = legacy_linked && task.collection_id == s.collections.front().id;
    }
    check(legacy_linked, "legacy tasks linked to Inbox");

    const int tasks_before = static_cast<int>(s.tasks.size());
    const int habits_before = static_cast<int>(s.habits.size());

    // Mutations.
    s.add_collection("Work");
    const int work_id = s.active_collection_id();
    s.open_selected_collection();
    s.add_task("Test task A");
    s.add_habit("Test habit A");
    s.selected_column = 0; // Todo column
    s.selected_card = 0;
    s.move_task_right(); // move focused Todo card to In Progress
    const Task* moved = s.focused_task();
    const Priority start_prio = moved ? moved->priority : Priority::None;
    const int moved_id = moved ? moved->id : -1;
    s.cycle_focused_priority();
    s.cycle_focused_priority();
    s.cycle_focused_priority();
    const Priority expected_prio = next_priority(next_priority(next_priority(start_prio)));
    s.set_focused_due("7"); // due in a week
    s.selected_habit = static_cast<int>(s.habits.size()) - 1;
    s.toggle_habit_today();
    check(s.dirty, "dirty flag set after mutations");

    // Save and reload into a fresh state.
    check(save_state(tmp, s), "save to temp file");

    AppState r;
    check(load_state(tmp, r, /*rebase=*/false), "reload saved file");
    check(r.dirty == false, "dirty cleared on reload");

    check(static_cast<int>(r.tasks.size()) == tasks_before + 1, "task count +1");
    check(r.collections.size() == 2, "collection count round-tripped");
    check(static_cast<int>(r.habits.size()) == habits_before + 1, "habit count +1");
    check(r.tasks.back().title == "Test task A", "added task title round-tripped");
    check(r.tasks.back().collection_id == work_id, "task collection round-tripped");
    check(r.habits.back().name == "Test habit A", "added habit name round-tripped");

    // Find the moved task: the first Todo card was moved to In Progress. Its id
    // is whichever task was at column 0 index 0 before the move. We identify it
    // by checking at least one in_progress task now has high priority + a due.
    bool found_moved = false;
    for (const auto& t : r.tasks) {
        if (t.id == moved_id
            && t.status == TaskStatus::InProgress
            && t.priority == expected_prio
            && !t.due_date.empty()) {
            found_moved = true;
        }
    }
    check(found_moved, "moved task kept priority + due");

    // Toggle persisted: the last habit should be done today.
    check(r.habits.back().done_on(iso_today()), "habit toggle persisted");

    // Search filter: the added task matches its own title, not "zzz".
    s.search_query = "Test task A";
    int col0 = 0;
    for (int c = 0; c < 3; ++c) col0 += static_cast<int>(s.column_tasks(c).size());
    s.search_query = "zzz";
    int colz = 0;
    for (int c = 0; c < 3; ++c) colz += static_cast<int>(s.column_tasks(c).size());
    check(col0 == 1 && colz == 0, "search filter matches selectively");

    r.selected_collection_id = work_id;
    r.delete_selected_collection();
    check(r.collections.size() == 1, "collection deleted");
    bool work_tasks_removed = true;
    for (const auto& task : r.tasks) {
        work_tasks_removed = work_tasks_removed && task.collection_id != work_id;
    }
    check(work_tasks_removed, "deleted collection tasks removed");
    r.delete_selected_collection();
    check(r.collections.size() == 1, "last collection protected");

    std::cout << (failures ? "\nFAILED tests\n" : "\nALL TESTS PASSED\n");
    return failures ? 1 : 0;
}
