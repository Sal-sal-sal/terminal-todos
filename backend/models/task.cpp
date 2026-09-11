#include "backend/models/task.hpp"

namespace term_todos {

int Task::column_index(TaskStatus status) {
    switch (status) {
        case TaskStatus::Todo:       return 0;
        case TaskStatus::InProgress: return 1;
        case TaskStatus::Done:       return 2;
    }
    return 0;
}

TaskStatus Task::column_to_status(int column) {
    switch (column) {
        case 0:  return TaskStatus::Todo;
        case 1:  return TaskStatus::InProgress;
        default: return TaskStatus::Done;
    }
}

const char* task_status_label(TaskStatus status) {
    switch (status) {
        case TaskStatus::Todo:       return "Todo";
        case TaskStatus::InProgress: return "In Progress";
        case TaskStatus::Done:       return "Done";
    }
    return "Todo";
}

Priority priority_from_string(const std::string& value) {
    if (value == "high")   return Priority::High;
    if (value == "medium") return Priority::Medium;
    if (value == "low")    return Priority::Low;
    return Priority::None;
}

const char* priority_to_string(Priority priority) {
    switch (priority) {
        case Priority::High:   return "high";
        case Priority::Medium: return "medium";
        case Priority::Low:    return "low";
        case Priority::None:   return "none";
    }
    return "none";
}

const char* priority_marker(Priority priority) {
    switch (priority) {
        case Priority::High:   return "H";
        case Priority::Medium: return "M";
        case Priority::Low:    return "L";
        case Priority::None:   return "";
    }
    return "";
}

Priority next_priority(Priority priority) {
    switch (priority) {
        case Priority::None:   return Priority::Low;
        case Priority::Low:    return Priority::Medium;
        case Priority::Medium: return Priority::High;
        case Priority::High:   return Priority::None;
    }
    return Priority::None;
}

} // namespace term_todos
