#include "src/cli.hpp"

#include "backend/controllers/app_state.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using term_todos::AppState;
using term_todos::CliAction;
using term_todos::CliOptions;
using term_todos::Priority;
using term_todos::Task;
using term_todos::TaskStatus;

namespace {

void check(bool condition, const std::string& message) {
    if (condition) return;
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
}

CliOptions parse(std::vector<std::string> values) {
    std::vector<char*> argv;
    for (auto& value : values) argv.push_back(value.data());
    return term_todos::parse_cli(static_cast<int>(argv.size()), argv.data());
}

void test_required_commands() {
    check(parse({"todo"}).action == CliAction::Run, "todo opens the app");
    check(parse({"todo", "help"}).action == CliAction::Help, "todo help");
    check(parse({"todo", "-h"}).action == CliAction::Help, "todo -h");
    check(parse({"todo", "--help"}).action == CliAction::Help, "todo --help");
    const CliOptions list = parse({"todo", "list"});
    check(list.action == CliAction::List && list.data_path.empty(), "todo list");
}

void test_data_paths_and_errors() {
    const CliOptions run = parse({"todo", "/tmp/items.json"});
    check(run.action == CliAction::Run && run.data_path == "/tmp/items.json",
          "interactive data path");
    const CliOptions list = parse({"todo", "list", "/tmp/items.json"});
    check(list.action == CliAction::List && list.data_path == "/tmp/items.json",
          "list data path");
    check(!parse({"todo", "--bad"}).error.empty(), "unknown option error");
    check(!parse({"todo", "list", "a", "b"}).error.empty(), "extra list argument");
}

void test_output() {
    std::ostringstream help;
    term_todos::print_cli_help(help);
    check(help.str().find("todo list") != std::string::npos, "help lists commands");

    AppState state;
    state.collections = {{1, "Inbox"}};
    Task active{1, "Ship CLI", "Document commands", TaskStatus::Todo,
                {"cli"}, Priority::High, "2026-09-14", 1};
    Task done{2, "Write test", "", TaskStatus::Done, {}, Priority::None, "", 1};
    state.tasks = {active, done};
    std::ostringstream list;
    term_todos::print_task_list(list, state);
    check(list.str().find("Tasks: 1 todo, 0 in progress, 1 done") != std::string::npos,
          "list summary");
    check(list.str().find("[ ] #1 [H] Ship CLI") != std::string::npos,
          "todo row metadata");
    check(list.str().find("Document commands") != std::string::npos,
          "task description");
    check(list.str().find("[x] #2 Write test") != std::string::npos,
          "done row");
}

} // namespace

int main() {
    test_required_commands();
    test_data_paths_and_errors();
    test_output();
    std::cout << "cli tests passed\n";
    return 0;
}
