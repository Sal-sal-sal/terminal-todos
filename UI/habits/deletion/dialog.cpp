#include "UI/habits/deletion/dialog.hpp"

#include "UI/habits/styled_text.hpp"
#include "UI/theme.hpp"

#include <ftxui/screen/terminal.hpp>

#include <algorithm>
#include <limits>
#include <utility>

namespace term_todos {

ftxui::Element render_habit_deletion(const AppState& state,
                                    const ftxui::Element& input_box) {
    using namespace ftxui;
    const Habit* habit = state.pending_habit_deletion();
    if (!habit) return text("Habit no longer exists. Press Esc to cancel.");
    const bool final = state.modal == AppState::ModalKind::ConfirmDeleteHabit;
    const std::string count = std::to_string(state.habit_delete_history_count);
    const std::string records = count + " history record"
        + (state.habit_delete_history_count == 1 ? "" : "s");
    const int width = std::clamp(Terminal::Size().dimx - 12, 24, 60);
    Elements rows = {
        text(final ? "Step 2 of 2" : "Step 1 of 2") | color(theme::dim()),
        render_habit_text(habit->name, width, std::numeric_limits<int>::max(), true, true),
        separator(),
        paragraph("Will delete 1 habit and " + records + ".") | bold,
    };
    if (final) {
        rows.push_back(paragraph("Type " + count + ", then press Enter to delete.") | bold);
        rows.push_back(input_box);
    } else {
        rows.push_back(paragraph("Enter to continue to final confirmation."));
    }
    if (state.is_error_status) {
        rows.push_back(paragraph(state.status_message) | color(Color::RedLight));
    }
    rows.push_back(text("Esc cancels without deleting anything") | color(theme::dim()));
    return window(text(final ? "Final confirmation" : "Delete habit") | bold,
                  vbox(std::move(rows)) | size(WIDTH, EQUAL, width))
        | color(theme::warn()) | clear_under | center;
}

} // namespace term_todos
