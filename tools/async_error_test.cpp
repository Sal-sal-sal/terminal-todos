#include "UI/app_view.hpp"
#include "backend/controllers/app_state.hpp"
#include "backend/core/async_runner.hpp"
#include "backend/core/error_handler.hpp"

#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

using namespace term_todos;
using ftxui::Event;

int failures = 0;

void check(bool condition, const std::string& message) {
    if (condition) {
        std::cout << "  ok   " << message << "\n";
    } else {
        std::cout << "  FAIL " << message << "\n";
        ++failures;
    }
}

} // namespace

int main() {
    std::cout << "Running async + time_limit + error_handler tests...\n";

    // 1. safe_execute utility
    {
        bool error_handled = false;
        std::string captured_err;
        bool ok = safe_execute(
            []() { throw std::runtime_error("test error message"); },
            [&](const std::string& err) {
                error_handled = true;
                captured_err = err;
            });
        check(!ok && error_handled && captured_err == "test error message",
              "safe_execute catches std::exception and calls error handler");

        bool ok_success = safe_execute([]() { /* no-op */ });
        check(ok_success, "safe_execute succeeds on clean execution");
    }

    // 2. Synchronous run_with_timeout: success case
    {
        bool on_error_called = false;
        bool result = AsyncRunner::run_with_timeout(
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); },
            std::chrono::milliseconds(200),
            [&](const std::string&) { on_error_called = true; });
        check(result && !on_error_called, "run_with_timeout succeeds within time limit");
    }

    // 3. Synchronous run_with_timeout: error handling case
    {
        bool on_error_called = false;
        std::string err_msg;
        bool result = AsyncRunner::run_with_timeout(
            []() { throw std::runtime_error("computation failed"); },
            std::chrono::milliseconds(200),
            [&](const std::string& err) {
                on_error_called = true;
                err_msg = err;
            });
        check(!result && on_error_called && err_msg == "computation failed",
              "run_with_timeout catches exception and routes to error handler");
    }

    // 4. Synchronous run_with_timeout: timeout enforcement case
    {
        bool on_error_called = false;
        std::string err_msg;
        auto start = std::chrono::steady_clock::now();
        bool result = AsyncRunner::run_with_timeout(
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); },
            std::chrono::milliseconds(40),
            [&](const std::string& err) {
                on_error_called = true;
                err_msg = err;
            });
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();

        check(!result && on_error_called && elapsed < 200,
              "run_with_timeout aborts waiting when time limit is exceeded");
        check(err_msg.find("timed out") != std::string::npos,
              "run_with_timeout generates timeout error message");
    }

    // 5. Asynchronous run_async: success case
    {
        AsyncRunner runner;
        std::atomic<bool> success_called{false};
        std::atomic<bool> error_called{false};

        runner.run_async(
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); },
            std::chrono::milliseconds(200),
            [&]() { success_called.store(true); },
            [&](const std::string&) { error_called.store(true); });

        // Wait up to 300ms for async completion
        for (int i = 0; i < 30 && !success_called.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        check(success_called.load() && !error_called.load(),
              "run_async completes and invokes success callback");
    }

    // 6. Asynchronous run_async: exception error handler
    {
        AsyncRunner runner;
        std::atomic<bool> success_called{false};
        std::atomic<bool> error_called{false};
        std::string captured_err;

        runner.run_async(
            []() { throw std::runtime_error("async calculation crashed"); },
            std::chrono::milliseconds(200),
            [&]() { success_called.store(true); },
            [&](const std::string& err) {
                error_called.store(true);
                captured_err = err;
            });

        for (int i = 0; i < 30 && !error_called.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        check(!success_called.load() && error_called.load() && captured_err == "async calculation crashed",
              "run_async catches exceptions and routes to error handler");
    }

    // 7. Asynchronous run_async: time limit enforcement
    {
        AsyncRunner runner;
        std::atomic<bool> success_called{false};
        std::atomic<bool> error_called{false};
        std::string timeout_err;

        auto start = std::chrono::steady_clock::now();
        runner.run_async(
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(500)); },
            std::chrono::milliseconds(50),
            [&]() { success_called.store(true); },
            [&](const std::string& err) {
                error_called.store(true);
                timeout_err = err;
            });

        for (int i = 0; i < 30 && !error_called.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();

        check(!success_called.load() && error_called.load() && elapsed < 300,
              "run_async times out without hanging caller thread");
        check(timeout_err.find("timed out") != std::string::npos,
              "run_async passes timeout message to error handler");
    }

    // 8. AppState status and error handling flags
    {
        AppState state;
        state.set_busy("Working...");
        check(state.is_busy && !state.is_error_status && state.status_message == "Working...",
              "set_busy sets busy state correctly");

        state.set_error("Disk full");
        check(!state.is_busy && state.is_error_status && state.status_message == "Disk full",
              "set_error sets error state correctly");

        state.set_status("All good");
        check(!state.is_busy && !state.is_error_status && state.status_message == "All good",
              "set_status sets normal status correctly");

        state.clear_status();
        check(!state.is_busy && !state.is_error_status && state.status_message.empty(),
              "clear_status resets all status flags");
    }

    // 9. E2E UI resilience against invalid input and errors
    {
        AppState state;
        state.collections.push_back({1, "Inbox"});
        state.selected_collection_id = 1;
        state.task_view = AppState::TaskView::Board;
        Task t;
        t.id = 1;
        t.title = "Task 1";
        t.collection_id = 1;
        state.tasks.push_back(t);

        auto screen = ftxui::ScreenInteractive::FixedSize(80, 24);
        auto app = make_app(state, screen);

        // Try setting invalid date format
        app->OnEvent(Event::Character('u'));
        check(state.modal == AppState::ModalKind::SetDue, "due date modal opened");
        for (char c : std::string("invalid-date-format")) {
            app->OnEvent(Event::Character(c));
        }
        app->OnEvent(Event::Return);
        check(state.is_error_status, "invalid due date sets error status instead of crashing");
        check(!state.status_message.empty(), "error status message is presented to user");

        // Verify UI renders without crash
        app->Render();

        // Trigger async export key 'x'
        app->OnEvent(Event::Character('x'));
        check(state.is_busy, "'x' triggers async export without blocking UI");
        app->Render();

        // Try navigating while export is in flight
        bool handled = app->OnEvent(Event::Character('1'));
        check(handled && state.tab == AppState::Tab::Habits,
              "UI remains interactive while async operation is in progress");
    }

    std::cout << (failures ? "\nFAILED async error tests\n" : "\nALL ASYNC ERROR TESTS PASSED\n");
    return failures ? 1 : 0;
}
