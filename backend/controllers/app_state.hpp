#pragma once

#include "backend/models/collection.hpp"
#include "backend/models/habit.hpp"
#include "backend/models/task.hpp"
#include "backend/goals/model.hpp"

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace term_todos {

struct AppState {
    enum class Tab { Habits, Tasks, Accomplishments, Goals };
    enum class TaskView { Collections, Board };

    Tab tab = Tab::Tasks;
    TaskView task_view = TaskView::Collections;

    std::vector<TodoCollection> collections;
    std::vector<Task> tasks;
    std::vector<Habit> habits;
    GoalMap goals;
    struct Accomplishment {
        std::string title;
        bool done = false;
        std::string completed_on;
        bool archived_on(const std::string& today) const;
    };
    std::vector<Accomplishment> accomplishments;
    int selected_accomplishment = 0;
    int celebrating_accomplishment = -1;
    std::chrono::steady_clock::time_point celebration_started;
    std::vector<int> accomplishment_order(const std::string& today) const;
    void navigate_accomplishment(int delta);
    void add_accomplishment(const std::string& title);
    void rename_accomplishment(const std::string& title);
    void toggle_accomplishment();
    void delete_accomplishment();

    std::string data_path;
    bool dirty = false;
    bool read_only_data = false;

    int selected_collection_id = 0;
    int selected_column = 0;
    int selected_card = 0;
    int selected_sub_todo = 0;
    int selected_habit = 0;

    enum class ModalKind {
        None,
        AddCollection,
        RenameCollection,
        DeleteCollection,
        SearchCollections,
        AddTask,
        EditTask,
        EditDescription,
        AddSubTodo,
        SetDue,
        Search,
        AddHabit,
        DeleteHabit,
        ConfirmDeleteHabit,
        RenameHabit,
        EditHabitDescription,
        AddAccomplishment,
        RenameAccomplishment,
        AddGoal, AddGoalChild, EditGoal, DescribeGoal, LinkGoalHabit, AddGoalHabit,
        DeleteGoal, ConfirmDeleteGoal,
    };
    ModalKind modal = ModalKind::None;
    int edit_target_id = 0;
    std::size_t habit_delete_history_count = 0;

    std::string collection_query;
    std::string search_query;
    std::string status_message;
    bool is_error_status = false;
    bool is_busy = false;
    bool show_help = false;
    bool show_habit_description = false;
    int habit_detail_line = 0;

    void set_status(const std::string& message);
    void set_error(const std::string& error_message);
    void set_busy(const std::string& busy_message);
    void clear_status();

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
    void set_focused_description(const std::string& description);
    void navigate_sub_todo(int delta);
    void add_sub_todo(const std::string& title);
    void toggle_selected_sub_todo();
    void delete_selected_sub_todo();
    void cycle_focused_priority();
    void set_focused_due(const std::string& spec);
    std::vector<const Task*> column_tasks(int column) const;
    const Task* focused_task() const;

    void navigate_habit(int delta);
    void toggle_habit_today();
    void add_habit(const std::string& name);
    void delete_focused_habit();
    const Habit* pending_habit_deletion() const;
    void request_habit_deletion();
    void confirm_habit_deletion(const std::string& confirmation);
    void rename_focused_habit(const std::string& name);
    void set_habit_description(const std::string& description);
};

} // namespace term_todos
