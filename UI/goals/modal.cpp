#include "UI/goals/modal.hpp"

#include "UI/theme.hpp"
#include "backend/models/habit_markup.hpp"
#include <ftxui/screen/terminal.hpp>
#include <algorithm>

namespace term_todos {
using namespace ftxui;
using Kind = AppState::ModalKind;

bool is_goal_modal(Kind kind) { return kind >= Kind::AddGoal && kind <= Kind::ConfirmDeleteGoal; }

Element render_goal_modal(const AppState& state, const Element& input) {
    std::string title;
    switch (state.modal) {
        case Kind::AddGoal: title = "New goal"; break;
        case Kind::AddGoalChild: title = "New child task"; break;
        case Kind::EditGoal: title = "Rename goal or task"; break;
        case Kind::DescribeGoal: title = "Description (blank to clear)"; break;
        case Kind::AddGoalHabit: title = "New daily habit under this branch"; break;
        case Kind::LinkGoalHabit: title = "Link an existing habit by ID"; break;
        case Kind::DeleteGoal: title = "Delete branch / step 1 of 2"; break;
        case Kind::ConfirmDeleteGoal: title = "Final confirmation / step 2 of 2"; break;
        default: return text("");
    }
    Elements body;
    if (state.modal == Kind::DeleteGoal || state.modal == Kind::ConfirmDeleteGoal) {
        const auto* target = find_goal(state.goals, state.edit_target_id);
        body.push_back(paragraph(target ? target->title : "Branch no longer exists") | bold);
        body.push_back(paragraph("This deletes " + std::to_string(state.goals.deletion_count)
            + " goal/task records, including all descendants.") | color(theme::warn()));
        body.push_back(paragraph("Linked habits and their history will NOT be deleted.") | color(theme::dim()));
        if (state.modal == Kind::ConfirmDeleteGoal) {
            body.push_back(text("Type exactly " + std::to_string(state.goals.deletion_count) + ":"));
            body.push_back(input);
        }
    } else {
        if (state.modal == Kind::LinkGoalHabit) {
            Elements habits;
            for (std::size_t i = 0; i < state.habits.size(); ++i) {
                const auto& habit = state.habits[i];
                auto row = paragraph(std::to_string(habit.id) + "  " + plain_habit_text(habit.name, true));
                if (static_cast<int>(i) == state.goals.habit_cursor) row = row | inverted | focus;
                habits.push_back(row);
            }
            if (habits.empty()) habits.push_back(paragraph("No habits yet. Esc, then N creates one here."));
            body.push_back(vbox(habits) | vscroll_indicator | yframe | size(HEIGHT, LESS_THAN, 8));
            body.push_back(text("Up/Down or PgUp/PgDn; Enter links selection") | color(theme::dim()));
            body.push_back(text("Or type a habit ID:") | color(theme::dim()));
        }
        body.push_back(input);
    }
    if (state.is_error_status) body.push_back(paragraph(state.status_message) | color(theme::warn()));
    body.push_back(text("Enter to confirm / Esc to cancel") | color(theme::dim()));
    const int width = std::clamp(Terminal::Size().dimx - 12, 24, 64);
    return window(text(title) | bold | color(theme::accent()), vbox(body) | size(WIDTH, EQUAL, width))
        | clear_under | center;
}

} // namespace term_todos
