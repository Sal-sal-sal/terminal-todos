#include "UI/app_view.hpp"
#include "backend/goals/codecs.hpp"
#include "backend/goals/controller.hpp"
#include "backend/models/date.hpp"
#include "backend/store/json_codecs.hpp"

#include <ftxui/screen/screen.hpp>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <iostream>

namespace {
using namespace term_todos;
using ftxui::Event;

void require(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

void test_model_and_codec() {
    AppState state;
    add_goal(state, "Health", 0);
    add_goal(state, "Fitness", 1);
    add_goal(state, "Running", 2);
    add_goal(state, "Nutrition", 1);
    add_goal(state, "Research", 0);
    auto rows = goal_rows(state.goals);
    require(rows.size() == 5 && rows[2].node->id == 3 && rows[3].node->id == 4,
        "Goal traversal lost nested branches");
    require(rows[2].continuation.size() == 1 && rows[2].continuation[0], "Branch guide missing");
    state.goals.collapsed.insert(2);
    require(goal_rows(state.goals).size() == 4, "Folding did not hide descendants");
    require(goal_subtree(state.goals, 1).size() == 4, "Folded descendants escaped deletion count");
    state.goals.selected_id = 2;
    navigate_goal(state, 1);
    require(state.goals.selected_id == 4, "Navigation entered collapsed branch");
    const auto encoded = encode_state_records(state);
    AppState restored;
    require(decode_state_records(encoded, restored), "Goal round trip failed");
    require(restored.goals.nodes.size() == 5 && restored.goals.nodes[2].parent_id == 2,
        "Goal hierarchy changed on save");
    for (int mode = 0; mode < 4; ++mode) {
        auto invalid = encoded;
        if (mode == 0) invalid["goals"][0]["parent_id"] = 3;
        if (mode == 1) invalid["goals"][2]["parent_id"] = 999;
        if (mode == 2) invalid["goals"][1]["id"] = 1;
        if (mode == 3) invalid["goals"] = "invalid";
        GoalMap unchanged = state.goals;
        require(!decode_goals(invalid, unchanged), "Invalid goal graph accepted");
        require(unchanged.nodes.size() == 5, "Failed load modified goal map");
    }
    GoalMap deep;
    for (int id = 1; id <= 3000; ++id) deep.nodes.push_back({id, id - 1, "Task", "", false, 0});
    require(goal_rows(deep).size() == 3000, "Deep goal tree failed");
    require(goal_subtree(deep, 1).size() == 3000, "Deep deletion traversal failed");
}

void test_keyboard_and_render() {
    AppState state;
    state.collections.push_back({1, "Inbox"});
    Habit habit;
    habit.id = 7;
    habit.name = "Run daily";
    habit.set_done(iso_add_days(iso_today(), -1), true);
    state.habits.push_back(habit);
    auto screen = ftxui::ScreenInteractive::FixedSize(80, 24);
    auto app = make_app(state, screen);
    auto key = [&](char c) { require(app->OnEvent(Event::Character(c)), "Goal key not handled"); };
    auto type = [&](const std::string& value) { for (char c : value) key(c); app->OnEvent(Event::Return); };
    key('4');
    require(state.tab == AppState::Tab::Goals, "Goals tab did not open");
    key('n'); type("Become healthier");
    key('a'); type("Improve fitness");
    key('s'); type("Plan a workout");
    key(' ');
    require(state.goals.nodes[2].done && !state.goals.nodes[1].done,
        "Completion affected the wrong task");
    key('k'); key('H'); type("missing");
    require(state.modal == AppState::ModalKind::LinkGoalHabit, "Invalid habit link closed picker");
    type("7");
    require(state.goals.nodes.back().habit_id == 7, "Existing habit was not linked");
    key(' ');
    require(state.habits[0].done_on(iso_today()) && compute_streak(state.habits[0], iso_today()) == 2,
        "Linked habit did not update daily history/streak");
    key('N'); type("Walk daily");
    require(state.habits.size() == 2 && state.goals.nodes.back().habit_id == 8,
        "Daily habit did not appear in both views");
    for (int width : {80, 90}) {
        auto frame = ftxui::Screen::Create(ftxui::Dimension::Fixed(width), ftxui::Dimension::Fixed(24));
        ftxui::Render(frame, render_app(state));
        const auto output = frame.ToString();
        for (const char* label : {"Habits", "Tasks", "Accomplishments", "Goals", "1/2/3/4", "Plan a workout"})
            require(output.find(label) != std::string::npos, "Goal layout clipped a tab or task");
    }
    state.goals.selected_id = 1;
    key('d');
    require(state.goals.deletion_count == 5, "Deletion count did not include subtree");
    app->OnEvent(Event::Return);
    app->OnEvent(Event::Return);
    require(state.goals.nodes.size() == 5, "Empty confirmation deleted branch");
    type("4");
    require(state.goals.nodes.size() == 5, "Wrong confirmation deleted branch");
    app->OnEvent(Event::Escape);
    key('d'); app->OnEvent(Event::Return); type("5");
    require(state.goals.nodes.empty() && state.habits.size() == 2,
        "Deleting branch removed linked habits");
    app->OnEvent(Event::Tab);
    require(state.tab == AppState::Tab::Habits, "Tab did not wrap to Habits");
}

void test_deleted_habit_links() {
    AppState state;
    Habit habit;
    habit.id = 1;
    habit.name = "Old habit";
    habit.set_done(iso_today(), true);
    state.habits.push_back(habit);
    add_goal(state, "Linked task", 0, 1);
    state.delete_focused_habit();
    require(state.goals.nodes[0].habit_id == 0 && state.goals.nodes[0].done,
        "Habit deletion left a dangling link or lost completion");
    state.add_habit("New habit reusing ID");
    toggle_goal(state);
    require(!state.habits[0].done_on(iso_today()), "Reused habit ID rebound an old goal link");
    require(!state.goals.nodes[0].done, "Detached task could not be completed independently");
}
} // namespace

int main() {
    test_model_and_codec();
    test_keyboard_and_render();
    test_deleted_habit_links();
    std::cout << "PASS: goal hierarchy, validation, UI, habits and guarded deletion\n";
}
