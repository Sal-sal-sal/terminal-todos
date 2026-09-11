#pragma once

#include "backend/models/collection.hpp"
#include "backend/models/habit.hpp"
#include "backend/models/task.hpp"

#include <string>
#include <vector>

namespace term_todos {

struct AppState {
    enum class Tab { Habits, Tasks };
    enum class TaskView { Collections, Board };

    Tab tab = Tab::Tasks;
    TaskView task_view = TaskView::Collections;

    std::vector<TodoCollection> collections;
    std::vector<Task> tasks;
    std::vector<Habit> habits;

    std::string data_path;
    bool dirty = false;

    int selected_collection_id = 0;
    int selected_column = 0;
    int selected_card = 0;
    int selected_habit = 0;

    enum class ModalKind {
        None,
        AddCollection,
        RenameCollection,
        DeleteCollection,
        SearchCollections,
        AddTask,
        EditTask,
        SetDue,
        Search,
        AddHabit,
        RenameHabit,
    };
    ModalKind modal = ModalKind::None;
    int edit_target_id = 0;

    std::string collection_query;
    std::string search_query;
    std::string status_message;
    bool show_help = false;

    void set_tab(Tab next);
    void clamp_selection();

    void navigate_collection(int delta);
    void open_selected_collection();
    void back_to_collections();
    void add_collection(const std::string& name);
    void rename_selected_collection(const std::string& name);
    void delete_selected_collection();
    int active_collection_id() const;
    const TodoCollection* selected_collection() const;
    std::vector<const TodoCollection*> visible_collections() const;

    void move_task_left();
    void move_task_right();
    void navigate_card(int delta);
    void navigate_column(int delta);
    void add_task(const std::string& title);
    void delete_focused_task();
    void rename_focused_task(const std::string& title);
    void cycle_focused_priority();
    void set_focused_due(const std::string& spec);
    std::vector<const Task*> column_tasks(int column) const;
    const Task* focused_task() const;

    void navigate_habit(int delta);
    void toggle_habit_today();
    void add_habit(const std::string& name);
    void delete_focused_habit();
    void rename_focused_habit(const std::string& name);
};

} // namespace term_todos
