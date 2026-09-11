#pragma once

#include "backend/core/error_handler.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace term_todos {

class AsyncRunner {
public:
    AsyncRunner();
    ~AsyncRunner();

    // Non-copyable, non-movable to ensure safe concurrency.
    AsyncRunner(const AsyncRunner&) = delete;
    AsyncRunner& operator=(const AsyncRunner&) = delete;
    AsyncRunner(AsyncRunner&&) = delete;
    AsyncRunner& operator=(AsyncRunner&&) = delete;

    // Runs a background task asynchronously with a time limit and error handling.
    // - task: Callable to execute on a background thread.
    // - time_limit: Max duration allowed before timing out (e.g. 3000ms).
    // - on_success: Callback invoked upon successful completion before timeout.
    // - on_error: Callback invoked if task throws an exception or times out.
    // - post_to_ui: Optional marshaler to dispatch callbacks on the UI loop thread
    //               (e.g. screen.Post([cb]{ cb(); })). If null, callbacks run directly.
    void run_async(
        std::function<void()> task,
        std::chrono::milliseconds time_limit,
        std::function<void()> on_success,
        ErrorHandler on_error,
        std::function<void(std::function<void()>)> post_to_ui = nullptr);

    // Executes a task synchronously with a strict timeout and exception handling.
    // Returns true on success, or false if it timed out or threw an exception.
    static bool run_with_timeout(
        const std::function<void()>& task,
        std::chrono::milliseconds time_limit,
        const ErrorHandler& on_error = nullptr);

    // Cancels all currently active/pending tasks and discards their completion callbacks.
    void cancel_all();

    // Returns the number of currently active background tasks.
    std::size_t active_tasks_count() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Global default AsyncRunner instance.
AsyncRunner& default_async_runner();

} // namespace term_todos
