#include "UI/tab_bar.hpp"

#include "UI/theme.hpp"

#include <ftxui/dom/elements.hpp>

namespace term_todos {

namespace {

ftxui::Element tab_item(const std::string& name, bool active) {
    auto label = ftxui::text(name);
    if (active) {
        label |= ftxui::bold;
        label |= ftxui::color(theme::accent());
    } else {
        label |= ftxui::color(theme::muted());
    }
    return label | ftxui::hcenter | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 12);
}

} // namespace

ftxui::Element render_tab_bar(const AppState& state) {
    using namespace ftxui;
    const bool on_habits = state.tab == AppState::Tab::Habits;
    auto habits = tab_item("Habits", on_habits);
    auto tasks = tab_item("Tasks", !on_habits);

    auto bar = hbox({
        text(" term-todos ") | bold | color(theme::accent()),
        separator(),
        habits,
        separator(),
        tasks,
        filler(),
        text("1/2 tabs ") | color(theme::dim()),
    });

    return bar | border;
}

} // namespace term_todos
