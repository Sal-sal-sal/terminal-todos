#include "UI/accomplishments/view.hpp"
#include "UI/theme.hpp"
#include <algorithm>

namespace term_todos {
using namespace ftxui;

Element render_accomplishments(const AppState& state) {
    const int done = std::count_if(state.accomplishments.begin(), state.accomplishments.end(),
        [](const auto& item) { return item.done; });
    Elements rows;
    for (int i = 0; i < static_cast<int>(state.accomplishments.size()); ++i) {
        const auto& item = state.accomplishments[i];
        auto title = paragraph(item.title);
        if (item.done) title = title | strikethrough | color(theme::muted());
        else title = title | bold;
        auto row = hbox({text(item.done ? "  ✓  " : "  ○  ") | color(Color::Yellow), title | flex});
        if (i == state.selected_accomplishment) row = row | focus | inverted;
        rows.push_back(row);
        rows.push_back(text(""));
    }
    if (rows.empty()) rows.push_back(text("  Press n to add your first accomplishment.") | color(theme::muted()));
    return vbox({
        text(""),
        text("  ACCOMPLISHMENTS") | bold | color(Color::Yellow),
        text("  " + std::to_string(done) + " / " + std::to_string(state.accomplishments.size()) + " completed") | color(theme::muted()),
        text(""),
        vbox(rows) | vscroll_indicator | yframe | flex,
    });
}

bool handle_accomplishment_event(AppState& state, const Event& event, std::string& input) {
    if (event == Event::ArrowUp || event == Event::Character('k')) state.navigate_accomplishment(-1);
    else if (event == Event::ArrowDown || event == Event::Character('j')) state.navigate_accomplishment(1);
    else if (event == Event::Character(' ') || event == Event::Return) state.toggle_accomplishment();
    else if (event == Event::Character('d')) state.delete_accomplishment();
    else if (event == Event::Character('n')) {
        input.clear();
        state.modal = AppState::ModalKind::AddAccomplishment;
    } else if (event == Event::Character('e')) {
        if (!state.accomplishments.empty()) {
            state.clamp_selection();
            input = state.accomplishments[state.selected_accomplishment].title;
            state.modal = AppState::ModalKind::RenameAccomplishment;
        }
    } else return false;
    return true;
}
}
