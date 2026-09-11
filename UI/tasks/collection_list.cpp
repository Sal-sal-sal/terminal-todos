#include "UI/tasks/collection_list.hpp"

#include "UI/theme.hpp"

#include <ftxui/dom/elements.hpp>

#include <sstream>

namespace term_todos {

using namespace ftxui;

namespace {

Element collection_row(const AppState& state, const TodoCollection& collection) {
    int total = 0;
    int done = 0;
    for (const auto& task : state.tasks) {
        if (task.collection_id != collection.id) continue;
        ++total;
        if (task.status == TaskStatus::Done) ++done;
    }
    const int percent = total == 0 ? 0 : done * 100 / total;
    std::ostringstream stats;
    stats << total << " tasks  -  " << done << " done  (" << percent << "%)";

    const bool focused = collection.id == state.selected_collection_id;
    auto row = hbox({
        text(focused ? " > " : "   ") | color(theme::accent()),
        text(collection.name) | bold | flex,
        text(stats.str()) | color(theme::dim()),
        text("  "),
    });
    if (focused) return row | borderDouble | color(theme::accent());
    return row | border | color(theme::muted());
}

Element empty_state(const AppState& state) {
    const std::string title = state.collections.empty()
        ? "No collections yet."
        : "No matching collections.";
    const std::string action = state.collections.empty()
        ? "Press n to create your first collection."
        : "Press f to change or clear the filter.";
    return vbox({text(title) | bold, text(""), text(action) | color(theme::dim())})
        | center | flex;
}

} // namespace

Element render_collection_list(const AppState& state) {
    Elements rows;
    rows.push_back(hbox({
        text(" TASK COLLECTIONS ") | bold | color(theme::accent()),
        text(std::to_string(state.collections.size())) | color(theme::dim()),
        filler(),
    }));
    if (!state.collection_query.empty()) {
        rows.push_back(text(" filter: \"" + state.collection_query + "\"")
                       | color(theme::accent()));
    }
    rows.push_back(separator());

    const auto visible = state.visible_collections();
    if (visible.empty()) {
        rows.push_back(empty_state(state));
    } else {
        for (const auto* collection : visible) {
            rows.push_back(collection_row(state, *collection));
        }
        rows.push_back(filler());
    }
    return vbox(rows) | flex;
}

} // namespace term_todos
