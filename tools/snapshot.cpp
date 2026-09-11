// Non-interactive render harness: loads the bundled mock and prints a fixed-
// size ASCII snapshot of the Tasks and Habits tabs. Used for pixel-perfect UI
// review and as a regression check that the layout compiles and renders.
//
// Build: cmake already builds a `term-todos-snapshot` target. Run with a data
// file path as argv[1] (defaults to the bundled mock).

#include "UI/app_view.hpp"
#include "backend/controllers/app_state.hpp"
#include "backend/store/json_store.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>

#include <iostream>
#include <cstdlib>
#include <string>

#ifndef TERM_TODOS_DATA_DIR
#define TERM_TODOS_DATA_DIR "."
#endif

namespace {

void snapshot(const term_todos::AppState& state, const std::string& label, int w, int h) {
    ftxui::Screen screen = ftxui::Screen::Create(
        ftxui::Dimension::Fixed(w), ftxui::Dimension::Fixed(h));
    ftxui::Render(screen, term_todos::render_app(state));

    std::cout << "===== " << label << " (" << w << "x" << h << ") =====\n";
    std::cout << screen.ToString();
    std::cout << "\n";
}

} // namespace

int main(int argc, char** argv) {
    using namespace term_todos;

    const std::string path = (argc > 1) ? argv[1]
                                        : std::string(TERM_TODOS_DATA_DIR) + "/mock.json";

    AppState state;
    if (!load_state(path, state, /*rebase=*/true)) {
        std::cerr << "snapshot: could not load " << path << "\n";
        return 1;
    }

    state.set_tab(AppState::Tab::Tasks);
    snapshot(state, "Collections", 90, 26);
    state.open_selected_collection();
    if (const char* q = std::getenv("TERM_TODOS_SEARCH")) {
        state.search_query = q;
        state.clamp_selection();
    }
    snapshot(state, "Collection board", 90, 26);
    state.search_query.clear();

    state.set_tab(AppState::Tab::Habits);
    snapshot(state, "Habits", 90, 26);

    if (const char* out = std::getenv("TERM_TODOS_EXPORT")) {
        if (!term_todos::export_habits_csv(out, state, 30)) {
            std::cerr << "snapshot: export failed\n";
            return 1;
        }
        std::cerr << "snapshot: exported to " << out << "\n";
    }

    return 0;
}
