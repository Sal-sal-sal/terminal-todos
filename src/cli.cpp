#include "src/cli.hpp"

#include "backend/controllers/app_state.hpp"
#include "backend/models/task.hpp"

#include <algorithm>
#include <array>
#include <ostream>
#include <string>

namespace term_todos {
namespace {

struct TaskCounts {
    int todo = 0;
    int in_progress = 0;
    int done = 0;
};

bool is_help(const std::string& value) {
    return value == "help" || value == "-h" || value == "--help";
}

const char* status_marker(TaskStatus status) {
    switch (status) {
        case TaskStatus::Todo: return " ";
        case TaskStatus::InProgress: return "~";
        case TaskStatus::Done: return "x";
    }
    return " ";
}

TaskCounts count_tasks(const AppState& state) {
    TaskCounts counts;
    for (const auto& task : state.tasks) {
        switch (task.status) {
            case TaskStatus::Todo: ++counts.todo; break;
            case TaskStatus::InProgress: ++counts.in_progress; break;
            case TaskStatus::Done: ++counts.done; break;
        }
    }
    return counts;
}

void print_task(std::ostream& out, const Task& task) {
    out << "  [" << status_marker(task.status) << "] #" << task.id;
    const char* priority = priority_marker(task.priority);
    if (priority[0] != '\0') out << " [" << priority << "]";
    out << " " << task.title;
    if (!task.due_date.empty()) out << "  due " << task.due_date;
    if (!task.labels.empty()) {
        out << "  [";
        for (std::size_t i = 0; i < task.labels.size(); ++i) {
            if (i > 0) out << ", ";
            out << task.labels[i];
        }
        out << "]";
    }
    out << "\n";
}

} // namespace

CliOptions parse_cli(int argc, char** argv) {
    CliOptions options;
    if (argc <= 1) return options;
    const std::string first = argv[1];
    if (is_help(first)) {
        options.action = CliAction::Help;
        if (argc > 2) options.error = "help does not accept arguments";
        return options;
    }
    if (first == "list") {
        options.action = CliAction::List;
        if (argc == 3) options.data_path = argv[2];
        if (argc > 3) options.error = "list accepts at most one data file";
        return options;
    }
    if (!first.empty() && first.front() == '-') {
        options.error = "unknown option: " + first;
        return options;
    }
    options.data_path = first;
    if (argc > 2) options.error = "expected at most one data file";
    return options;
}

void print_cli_help(std::ostream& out) {
    out << "term-todos - terminal task and habit tracker\n\n"
        << "Usage:\n"
        << "  todo                         Open the interactive interface\n"
        << "  todo [data-file]             Open a specific data file\n"
        << "  todo list [data-file]        Print tasks and exit\n"
        << "  todo help                    Show this help\n"
        << "  todo -h | --help             Show this help\n\n"
        << "Task markers: [ ] todo, [~] in progress, [x] done\n";
}

void print_task_list(std::ostream& out, const AppState& state) {
    const TaskCounts counts = count_tasks(state);
    out << "Tasks: " << counts.todo << " todo, " << counts.in_progress
        << " in progress, " << counts.done << " done\n";
    if (state.tasks.empty()) {
        out << "\nNo tasks.\n";
        return;
    }
    const std::array<TaskStatus, 3> statuses = {
        TaskStatus::Todo, TaskStatus::InProgress, TaskStatus::Done,
    };
    for (const auto& collection : state.collections) {
        const int total = static_cast<int>(std::count_if(
            state.tasks.begin(), state.tasks.end(), [&](const Task& task) {
                return task.collection_id == collection.id;
            }));
        if (total == 0) continue;
        out << "\n" << collection.name << " (" << total << ")\n";
        for (TaskStatus status : statuses) {
            for (const auto& task : state.tasks) {
                if (task.collection_id == collection.id && task.status == status) {
                    print_task(out, task);
                }
            }
        }
    }
}

} // namespace term_todos
