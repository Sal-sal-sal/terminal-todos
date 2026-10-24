#include "UI/overlay.hpp"

#include "UI/theme.hpp"

namespace term_todos {

using namespace ftxui;

namespace {

Element help_row(const char* key, const char* description) {
    return hbox({
        text(key) | color(theme::accent()) | size(WIDTH, EQUAL, 12),
        text(description) | color(theme::muted()),
    });
}

Element help_heading(const char* title, Color tint = Color::GrayLight) {
    return text(title) | bold | color(tint);
}

} // namespace

Element render_help() {
    auto left = vbox({
        help_heading("GLOBAL"),
        help_row("1/2/3/4", "switch tabs"),
        help_row("Tab", "next tab"),
        help_row("? / Esc", "close help"),
        help_row("x", "export habits"),
        help_row("q", "save and quit"),
        separator(),
        help_heading("COLLECTIONS"),
        help_row("j / k", "select"),
        help_row("Enter / l", "open"),
        help_row("n / e / d", "new / edit / delete"),
        help_row("f", "find"),
        separator(),
        help_heading("TASK BOARD"),
        help_row("h/l  j/k", "move / select"),
        help_row("n/e/D/d", "new / name / desc / del"),
        help_row("s/J/K/X", "sub-todos"),
        help_row("Space [/]", "toggle / priority"),
        help_row("u / b / f", "due / back / find"),
    }) | size(WIDTH, EQUAL, 36);

    auto right = vbox({
        help_heading("HABITS", Color::GreenLight),
        help_row("j/k / Space", "select / toggle"),
        help_row("n / e", "new / name"),
        help_row("d", "delete (2 steps)"),
        help_row("D / Enter", "description / details"),
        help_row("PgUp/PgDn", "scroll; Home/End edges"),
        separator(),
        help_heading("COLOR TAGS", Color::RGB(255, 166, 74)),
        help_row("[#red]", "red text"),
        help_row("[#green]", "green text"),
        help_row("[#orange]", "orange text"),
        help_row("[#blue]", "blue text"),
        help_row("[#end]", "normal text"),
        help_row("[word]", "old 3-part style"),
        separator(),
        help_heading("ACCOMPLISHMENTS", Color::Yellow),
        help_row("j/k / Space", "select / toggle"),
        help_row("Enter", "toggle too"),
        help_row("n / e / d", "new / edit / delete"),
    }) | size(WIDTH, EQUAL, 36);

    auto body = vbox({
        text("KEYBOARD + COLOR GUIDE") | bold | color(theme::accent()),
        separator(),
        hbox({left, separator(), right}),
        text("Press ? or Esc to close") | color(theme::dim()),
    });
    return body | border | color(theme::muted()) | clear_under | center;
}

} // namespace term_todos
