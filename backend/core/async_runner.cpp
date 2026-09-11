#include "backend/core/async_runner.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace term_todos {

namespace {

struct TaskControlBlock {
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> completed{false};
    std::atomic<bool> timed_out{false};
    std::atomic<bool> cancelled{false};
    std::function<void()> on_success;
    ErrorHandler on_error;
    std::function<void(std::function<void()>)> post_to_ui;

    void dispatch_success() {
        if (cancelled.load() || timed_out.load()) return;
        auto cb = on_success;
        auto post = post_to_ui;
        if (!cb) return;
        if (post) {
            post([cb = std::move(cb)]() { cb(); });
        } else {
            cb();
        }
    }

    void dispatch_error(const std::string& err) {
        if (cancelled.load()) return;
        auto cb = on_error;
        auto post = post_to_ui;
        if (!cb) return;
        if (post) {
            post([cb = std::move(cb), err]() { cb(err); });
        } else {
            cb(err);
        }
    }
};

} // namespace

struct AsyncRunner::Impl {
    mutable std::mutex list_mutex;
    std::vector<std::shared_ptr<TaskControlBlock>> active_tasks;

    void cleanup_finished() {
        std::lock_guard<std::mutex> lock(list_mutex);
        active_tasks.erase(
            std::remove_if(active_tasks.begin(), active_tasks.end(),
                [](const std::shared_ptr<TaskControlBlock>& c) {
                    return c->completed.load() || c->timed_out.load() || c->cancelled.load();
                }),
            active_tasks.end());
    }

    void cancel_all() {
        std::lock_guard<std::mutex> lock(list_mutex);
        for (auto& task : active_tasks) {
            task->cancelled.store(true);
            task->cv.notify_all();
        }
        active_tasks.clear();
    }
};

AsyncRunner::AsyncRunner() : impl_(std::make_unique<Impl>()) {}

AsyncRunner::~AsyncRunner() {
    cancel_all();
}

void AsyncRunner::cancel_all() {
    if (impl_) {
        impl_->cancel_all();
    }
}

std::size_t AsyncRunner::active_tasks_count() const {
    if (!impl_) return 0;
    std::lock_guard<std::mutex> lock(impl_->list_mutex);
    return impl_->active_tasks.size();
}

void AsyncRunner::run_async(
    std::function<void()> task,
    std::chrono::milliseconds time_limit,
    std::function<void()> on_success,
    ErrorHandler on_error,
    std::function<void(std::function<void()>)> post_to_ui) {

    impl_->cleanup_finished();

    auto control = std::make_shared<TaskControlBlock>();
    control->on_success = std::move(on_success);
    control->on_error = std::move(on_error);
    control->post_to_ui = std::move(post_to_ui);

    {
        std::lock_guard<std::mutex> lock(impl_->list_mutex);
        impl_->active_tasks.push_back(control);
    }

    // Worker thread executing the task
    std::thread([control, task = std::move(task)]() {
        bool failed = false;
        std::string err_msg;
        try {
            task();
        } catch (const std::exception& e) {
            failed = true;
            err_msg = e.what();
        } catch (...) {
            failed = true;
            err_msg = "Unknown error occurred";
        }

        {
            std::lock_guard<std::mutex> lock(control->mutex);
            if (!control->timed_out.load() && !control->cancelled.load()) {
                control->completed.store(true);
                if (failed) {
                    control->dispatch_error(err_msg);
                } else {
                    control->dispatch_success();
                }
            }
        }
        control->cv.notify_all();
    }).detach();

    // Watchdog timer thread when time_limit > 0
    if (time_limit.count() > 0) {
        std::thread([control, time_limit]() {
            std::unique_lock<std::mutex> lock(control->mutex);
            if (!control->cv.wait_for(lock, time_limit, [&]() {
                return control->completed.load() || control->cancelled.load();
            })) {
                // Timed out
                if (!control->cancelled.load() && !control->completed.load()) {
                    control->timed_out.store(true);
                    control->dispatch_error("Operation timed out after " +
                                            std::to_string(time_limit.count()) + "ms");
                }
            }
        }).detach();
    }
}

bool AsyncRunner::run_with_timeout(
    const std::function<void()>& task,
    std::chrono::milliseconds time_limit,
    const ErrorHandler& on_error) {

    auto done_promise = std::make_shared<std::promise<void>>();
    auto done_future = done_promise->get_future();
    auto threw = std::make_shared<std::atomic<bool>>(false);
    auto err_msg = std::make_shared<std::string>();

    std::thread worker([task, done_promise, threw, err_msg]() {
        try {
            task();
            done_promise->set_value();
        } catch (const std::exception& e) {
            threw->store(true);
            *err_msg = e.what();
            try { done_promise->set_value(); } catch (...) {}
        } catch (...) {
            threw->store(true);
            *err_msg = "Unknown error occurred";
            try { done_promise->set_value(); } catch (...) {}
        }
    });

    if (time_limit.count() > 0 && done_future.wait_for(time_limit) == std::future_status::timeout) {
        // Timed out: detach worker so caller thread doesn't hang
        worker.detach();
        if (on_error) {
            on_error("Operation timed out after " + std::to_string(time_limit.count()) + "ms");
        }
        return false;
    }

    // Finished within time limit
    worker.join();
    if (threw->load()) {
        if (on_error) {
            on_error(*err_msg);
        }
        return false;
    }
    return true;
}

AsyncRunner& default_async_runner() {
    static AsyncRunner runner;
    return runner;
}

} // namespace term_todos
