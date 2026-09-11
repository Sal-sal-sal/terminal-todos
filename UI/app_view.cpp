#include "UI/app_view.hpp"

#include "UI/event_handler.hpp"
#include "UI/habits/habits_view.hpp"
#include "UI/overlay.hpp"
#include "UI/tab_bar.hpp"
#include "UI/tasks/board_view.hpp"
#include "UI/tasks/collection_list.hpp"
#include "UI/theme.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace term_todos {

using namespace ftxui;

namespace {

// The contextual hint pairs shown in the footer. Keep labels short so the bar
// fits an 80-column terminal.
struct Hint {
    const char* key;
    const char* label;
};

Element hint_row(const Hint* hints, int count) {
    Elements parts;
    for (int i = 0; i < count; ++i) {
        parts.push_back(text(hints[i].key) | color(theme::accent()));
        parts.push_back(text(" ") | color(theme::dim()));
        parts.push_back(text(hints[i].label) | color(theme::dim()));
        parts.push_back(text("   "));
    }
    if (!parts.empty()) parts.pop_back();
    return hbox(parts);
}

Element footer(const AppState& state) {
    if (state.is_busy) {
        return hbox({
            text("⏳ ") | color(theme::accent()),
            text(state.status_message.empty() ? "Working..." : state.status_message)
                | color(theme::accent()) | bold,
        });
    }
    if (!state.status_message.empty()) {
        const Color msg_color = state.is_error_status ? Color::Red : theme::good();
        const std::string prefix = state.is_error_status ? "Error: " : "";
        return text(prefix + state.status_message) | color(msg_color) | bold;
    }
    if (state.tab == AppState::Tab::Tasks) {
        if (state.task_view == AppState::TaskView::Collections) {
            static const Hint hints[] = {
                {"j/k", "navigate"}, {"Enter/l", "open"}, {"n", "new"},
                {"e", "rename"}, {"d", "delete"}, {"f", "find"}, {"q", "quit"},
            };
            return hint_row(hints, sizeof(hints) / sizeof(hints[0]));
        }
        static const Hint hints[] = {
            {"h/l", "move"}, {"j/k", "select"}, {"n", "new"}, {"e", "edit"},
            {"d", "delete"}, {"f", "find"}, {"b", "collections"},
        };
        return hint_row(hints, sizeof(hints) / sizeof(hints[0]));
    }
    static const Hint hints[] = {
        {"j/k", "select"}, {"space", "toggle"}, {"n", "new"},
        {"e", "rename"}, {"d", "delete"},
    };
    return hint_row(hints, sizeof(hints) / sizeof(hints[0]));
}

Element content(const AppState& state) {
    if (state.tab == AppState::Tab::Tasks) {
        return state.task_view == AppState::TaskView::Collections
            ? render_collection_list(state)
            : render_board(state);
    }
    return render_habits(state);
}

} // namespace

Element render_app(const AppState& state) {
    return vbox({
        render_tab_bar(state),
        separator(),
        content(state) | flex,
        separator(),
        footer(state),
    });
}

Component make_app(AppState& state, ScreenInteractive& screen) {
    auto input_content = std::make_shared<std::string>();
    InputOption opt;
    opt.placeholder = "type here";
    opt.multiline = false;
    Component input = Input(input_content.get(), opt);

    auto root = Renderer(input, [&state, input] {
        try {
            Element body = render_app(state);
            if (state.show_help) {
                body = dbox({body, render_help()});
            } else if (state.modal != AppState::ModalKind::None) {
                body = dbox({body, render_modal(state, input->Render())});
            }
            return body;
        } catch (const std::exception& ex) {
            return vbox({
                text("Rendering Error: " + std::string(ex.what())) | color(Color::Red) | bold,
                text("Press any key to recover.") | color(theme::dim()),
            });
        } catch (...) {
            return vbox({
                text("Unknown Rendering Error") | color(Color::Red) | bold,
                text("Press any key to recover.") | color(theme::dim()),
            });
        }
    });

    root |= CatchEvent([&state, &screen, input, input_content](const Event& event) {
        try {
            if (state.show_help) {
                // Any key closes the help overlay.
                state.show_help = false;
                return true;
            }
            if (state.modal != AppState::ModalKind::None) {
                if (event == Event::Return) {
                    commit_modal(state, *input_content);
                    input_content->clear();
                    return true;
                }
                if (event == Event::Escape) {
                    state.modal = AppState::ModalKind::None;
                    input_content->clear();
                    return true;
                }
                // Route typing to the input box.
                return input->OnEvent(event);
            }
            return handle_event(state, event, screen, *input_content);
        } catch (const std::exception& ex) {
            state.set_error(std::string(ex.what()));
            return true;
        } catch (...) {
            state.set_error("An unexpected error occurred");
            return true;
        }
    });

    return root;
}

} // namespace term_todos
