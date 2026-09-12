#pragma once

#include <iosfwd>
#include <string>

namespace term_todos {

struct AppState;

enum class CliAction {
    Run,
    Help,
    List,
};

struct CliOptions {
    CliAction action = CliAction::Run;
    std::string data_path;
    std::string error;
};

CliOptions parse_cli(int argc, char** argv);
void print_cli_help(std::ostream& out);
void print_task_list(std::ostream& out, const AppState& state);

} // namespace term_todos
