#include "UI/app_view.hpp"
#include "backend/controllers/app_state.hpp"
#include "backend/core/async_runner.hpp"
#include "backend/store/json_store.hpp"

#include <ftxui/component/screen_interactive.hpp>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#ifndef TERM_TODOS_DATA_DIR
#define TERM_TODOS_DATA_DIR "."
#endif

namespace {

namespace fs = std::filesystem;

// Path to the bundled mock data, resolved at build time.
std::string bundled_mock_path() {
    return std::string(TERM_TODOS_DATA_DIR) + "/mock.json";
}

// Resolves the persistent user data file. Priority:
//   argv override (testing) -> $TERM_TODOS_DATA -> $XDG_DATA_HOME ->
//   ~/.local/share/term-todos/data.json.
// Returns an empty path if no writable home could be found (in which case the
// app falls back to the read-only bundled mock and will not persist).
std::string resolve_data_path(int argc, char** argv) {
    if (argc > 1) return argv[1];

    if (const char* env = std::getenv("TERM_TODOS_DATA")) {
        return env;
    }

    fs::path dir;
    if (const char* xdg = std::getenv("XDG_DATA_HOME")) {
        dir = fs::path(xdg);
    } else if (const char* home = std::getenv("HOME")) {
        dir = fs::path(home) / ".local" / "share";
    } else {
        return bundled_mock_path();
    }

    std::error_code ec;
    fs::create_directories(dir / "term-todos", ec);
    return (dir / "term-todos" / "data.json").string();
}

// On first run (no user data file yet) seed it from the bundled mock so the app
// starts with something to look at. The seed is loaded with rebase=true so the
// heatmap looks live.
void seed_if_missing(const std::string& data_path) {
    if (fs::exists(data_path)) return;

    term_todos::AppState seed;
    if (!term_todos::load_state(bundled_mock_path(), seed, /*rebase=*/true)) return;
    seed.data_path = data_path;
    seed.dirty = false;
    if (!term_todos::save_state(data_path, seed)) {
        std::perror("term-todos: seeding data");
    }
}

} // namespace

int main(int argc, char** argv) {
    using namespace term_todos;

    const std::string data_path = resolve_data_path(argc, argv);
    const bool is_bundled = (data_path == bundled_mock_path());

    // For the bundled fallback there is nothing to seed and no persistence.
    if (!is_bundled) seed_if_missing(data_path);

    AppState state;
    if (!load_state(data_path, state, /*rebase=*/false)) {
        std::cerr << "term-todos: could not load data from " << data_path << "\n";
        std::cerr << "Pass a JSON file path as the first argument.\n";
        return 1;
    }

    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
    ftxui::Component app = make_app(state, screen);
    screen.Loop(app);

    // Persist any unsaved mutations on a clean exit with timeout and error handling.
    if (state.dirty && !is_bundled) {
        bool saved = AsyncRunner::run_with_timeout(
            [&]() {
                if (!save_state(data_path, state)) {
                    throw std::runtime_error("could not write file " + data_path);
                }
            },
            /*time_limit=*/std::chrono::milliseconds(3000),
            /*on_error=*/[&](const std::string& err) {
                std::cerr << "term-todos: error saving data: " << err << "\n";
            }
        );
        if (!saved) {
            std::cerr << "term-todos: warning: unsaved changes could not be saved to " << data_path << "\n";
        }
    }
    return 0;
}
