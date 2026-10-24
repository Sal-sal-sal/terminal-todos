// Non-interactive render harness: loads the bundled mock and prints a fixed-
// size ASCII snapshot of the Tasks and Habits tabs. Used for pixel-perfect UI
// review and as a regression check that the layout compiles and renders.
//
// Build: cmake already builds a `term-todos-snapshot` target. Run with a data
// file path as argv[1] (defaults to the bundled mock).

#include "UI/app_view.hpp"
#include "UI/overlay.hpp"
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

void snapshot_element(const ftxui::Element& element,
                      const std::string& label, int w, int h) {
    ftxui::Screen screen = ftxui::Screen::Create(
        ftxui::Dimension::Fixed(w), ftxui::Dimension::Fixed(h));
    ftxui::Render(screen, element);

    std::cout << "===== " << label << " (" << w << "x" << h << ") =====\n";
    std::cout << screen.ToString();
    std::cout << "\n";
}

void snapshot(const term_todos::AppState& state, const std::string& label, int w, int h) {
    snapshot_element(term_todos::render_app(state), label, w, h);
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
    snapshot(state, "Habits narrow", 80, 24);
    state.set_tab(AppState::Tab::Accomplishments);
    snapshot(state, "Accomplishments", 80, 24);
    state.set_tab(AppState::Tab::Goals);
    snapshot(state, "Goals empty", 80, 24);
    state.goals.nodes = {
        {1, 0, "Become healthier", "Build fitness with small, repeatable steps.", false, 0},
        {2, 1, "Improve fitness", "", false, 0},
        {3, 2, "Plan a workout", "Choose three exercises and a realistic schedule.", true, 0},
        {4, 2, "Run every morning", "", false, state.habits.empty() ? 0 : state.habits.front().id},
        {5, 1, "Improve nutrition", "", false, 0},
        {6, 5, "Prepare a weekly meal plan", "", false, 0},
    };
    state.goals.selected_id = 3;
    snapshot(state, "Goals map", 80, 24);
    snapshot(state, "Goals map wide", 90, 26);
    snapshot_element(render_help(), "Help", 80, 24);

    AppState described;
    described.collections.push_back({1, "Inbox"});
    Habit example;
    example.id = 1;
    example.name = "I'm [#red]Reading[#end] in [#green]My Room[#end] at "
        "[#blue]8:00AM[#end] for thirty minutes before breakfast";
    example.description = "After [#green]breakfast[#end], read one chapter and write "
        "a short note about the idea that matters most.";
    described.habits.push_back(std::move(example));
    described.set_tab(AppState::Tab::Habits);
    snapshot(described, "Habit description preview", 80, 24);
    described.show_habit_description = true;
    snapshot(described, "Habit description full", 80, 24);

    described.habits.front().set_done("2026-01-01", true);
    described.request_habit_deletion();
    snapshot_element(render_modal(described, ftxui::text("")), "Habit deletion review", 80, 24);
    described.confirm_habit_deletion("");
    snapshot_element(render_modal(described, ftxui::text("type here")),
                     "Habit deletion final", 80, 24);

    if (const char* out = std::getenv("TERM_TODOS_EXPORT")) {
        if (!term_todos::export_habits_csv(out, state, 30)) {
            std::cerr << "snapshot: export failed\n";
            return 1;
        }
        std::cerr << "snapshot: exported to " << out << "\n";
    }

    return 0;
}
