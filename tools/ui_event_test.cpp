#include "UI/app_view.hpp"

#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <cstdlib>
#include <iostream>

namespace {

using namespace term_todos;
using ftxui::Event;

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void type(ftxui::Component& app, const std::string& text) {
    for (char c : text) require(app->OnEvent(Event::Character(c)), "typing failed");
}

void test_empty_column_navigation() {
    AppState state;
    state.collections.push_back({1, "Inbox"});
    state.selected_collection_id = 1;
    state.task_view = AppState::TaskView::Board;
    state.selected_column = 0;
    Task task;
    task.id = 1;
    task.title = "Already in progress";
    task.status = TaskStatus::InProgress;
    task.collection_id = 1;
    state.tasks.push_back(task);

    auto screen = ftxui::ScreenInteractive::FixedSize(80, 24);
    auto app = make_app(state, screen);
    require(app->OnEvent(Event::Character('l')), "right key was not handled");
    require(state.selected_column == 1, "could not leave empty Todo column");
    require(state.tasks.front().status == TaskStatus::InProgress,
            "navigation moved a task from another column");
    require(!state.dirty, "empty-column navigation marked state dirty");

    require(app->OnEvent(Event::Character('l')), "task move key was not handled");
    require(state.selected_column == 2, "focus did not follow moved task");
    require(state.tasks.front().status == TaskStatus::Done,
            "focused task did not move to Done");
}

} // namespace

int main() {
    test_empty_column_navigation();

    AppState state;
    state.collections.push_back({1, "Inbox"});
    state.selected_collection_id = 1;
    state.task_view = AppState::TaskView::Board;
    Task existing;
    existing.id = 1;
    existing.title = "Existing task";
    existing.collection_id = 1;
    state.tasks.push_back(existing);

    auto screen = ftxui::ScreenInteractive::FixedSize(80, 24);
    auto app = make_app(state, screen);

    require(app->OnEvent(Event::Character('e')), "edit key was not handled");
    require(state.modal == AppState::ModalKind::EditTask, "edit modal did not open");
    app->Render();
    require(app->OnEvent(Event::Return), "edit save was not handled");
    require(state.modal == AppState::ModalKind::None, "edit modal did not close");

    require(app->OnEvent(Event::Character('f')), "find key was not handled");
    require(state.modal == AppState::ModalKind::Search, "find modal did not open");
    app->Render();
    require(app->OnEvent(Event::Escape), "find cancel was not handled");
    require(state.modal == AppState::ModalKind::None, "find modal did not close");

    const auto old_tasks = state.tasks.size();
    require(app->OnEvent(Event::Character('n')), "new task key was not handled");
    type(app, "New");
    app->Render();
    require(app->OnEvent(Event::Return), "new task save was not handled");
    require(state.tasks.size() == old_tasks + 1, "new task was not created");
    require(state.tasks.back().title == "New", "new task title is incorrect");
    require(state.tasks.back().collection_id == 1, "task escaped active collection");

    require(app->OnEvent(Event::Character('b')), "back key was not handled");
    require(state.task_view == AppState::TaskView::Collections,
            "board did not return to collections");
    require(app->OnEvent(Event::Character('n')), "new collection key was not handled");
    type(app, "Work");
    require(app->OnEvent(Event::Return), "new collection save was not handled");
    require(state.collections.size() == 2, "collection was not created");
    require(state.selected_collection()->name == "Work", "new collection not selected");

    require(app->OnEvent(Event::Character('f')), "collection find was not handled");
    app->Render();
    require(app->OnEvent(Event::Escape), "collection find cancel failed");
    require(app->OnEvent(Event::Return), "open collection was not handled");
    require(state.task_view == AppState::TaskView::Board, "collection did not open");
    require(state.column_tasks(0).empty(), "collection leaked tasks from Inbox");

    std::cout << "ui event test passed\n";
    return 0;
}
