#include "UI/accomplishments/view.hpp"

#include "UI/theme.hpp"
#include "backend/models/date.hpp"

#include <ftxui/component/animation.hpp>

#include <chrono>

namespace term_todos {

using namespace ftxui;

namespace {

Element accomplishment_row(const AppState& state, int index, const std::string& today) {
    const auto& item = state.accomplishments[index];
    const bool archived = item.archived_on(today);
    const bool focused = index == state.selected_accomplishment;

    std::string marker = item.done ? "  ✓  " : "  ○  ";
    if (item.done && index == state.celebrating_accomplishment) {
        const auto elapsed = std::chrono::steady_clock::now() - state.celebration_started;
        if (elapsed < std::chrono::milliseconds(900)) {
            const int frame = static_cast<int>(
                std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 150);
            marker = frame % 2 == 0 ? "  ✦  " : "  ✧  ";
            animation::RequestAnimationFrame();
        }
    }

    auto title = paragraph(item.title);
    if (item.done) title = title | strikethrough | bold
        | color(archived ? Color::Green : Color::GreenLight);
    else title = title | bold;

    auto row = hbox({
        text(marker) | color(item.done ? Color::GreenLight : Color::Yellow),
        title | flex,
    });
    if (focused) {
        row = row | focus;
        row = item.done ? row | bgcolor(Color::RGB(24, 60, 39)) : row | inverted;
    }
    return row;
}

void append_section(Elements& rows, const AppState& state,
                    const std::vector<int>& indices, const std::string& today,
                    const std::string& heading, const std::string& empty_text,
                    Color heading_color) {
    rows.push_back(hbox({
        text("  " + heading) | bold | color(heading_color),
        text("  " + std::to_string(indices.size())) | color(theme::dim()),
    }));
    rows.push_back(text(""));
    if (indices.empty()) {
        rows.push_back(text("  " + empty_text) | color(theme::muted()));
    } else {
        for (int index : indices) {
            rows.push_back(accomplishment_row(state, index, today));
            rows.push_back(text(""));
        }
    }
}

} // namespace

Element render_accomplishments(const AppState& state) {
    const std::string today = iso_today();
    std::vector<int> current;
    std::vector<int> archived;
    int done_today = 0;
    for (int index : state.accomplishment_order(today)) {
        const auto& item = state.accomplishments[index];
        if (item.archived_on(today)) archived.push_back(index);
        else {
            current.push_back(index);
            if (item.done) ++done_today;
        }
    }

    Elements rows;
    append_section(rows, state, current, today, "ACCOMPLISHMENTS",
                   "Press n to add an accomplishment.", Color::Yellow);
    rows.push_back(separator());
    rows.push_back(text(""));
    append_section(rows, state, archived, today, "ACCOMPLISH COMPLETED",
                   "Finished work moves here tomorrow.", Color::GreenLight);

    return vbox({
        text(""),
        text("  " + std::to_string(done_today) + " completed today  •  "
             + std::to_string(archived.size()) + " from previous days")
            | color(theme::muted()),
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

} // namespace term_todos
