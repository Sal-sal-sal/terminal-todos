#pragma once

#include <string>
#include <vector>

namespace term_todos {

// Lifecycle of a task, mirroring GitHub Issues / Kanban columns.
enum class TaskStatus {
    Todo,
    InProgress,
    Done,
};

// Importance of a task. Used for sorting within a column and for the colored
// marker on the card.
enum class Priority {
    None,
    Low,
    Medium,
    High,
};

struct Task {
    int id = 0;
    std::string title;
    TaskStatus status = TaskStatus::Todo;
    std::vector<std::string> labels;
    Priority priority = Priority::None;
    std::string due_date; // optional ISO "YYYY-MM-DD"; empty means no deadline
    int collection_id = 1;

    // Maps a TaskStatus to the index of its Kanban column (0..2).
    static int column_index(TaskStatus status);
    static TaskStatus column_to_status(int column);
};

const char* task_status_label(TaskStatus status);

// String <-> enum conversions for (de)serialisation.
Priority priority_from_string(const std::string& value);
const char* priority_to_string(Priority priority);
// Short uppercase marker shown on the card, e.g. "H", "M", "L", or "".
const char* priority_marker(Priority priority);

// Cycle priority forward (None -> Low -> Medium -> High -> None).
Priority next_priority(Priority priority);

} // namespace term_todos
