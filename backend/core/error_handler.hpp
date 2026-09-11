#pragma once

#include <exception>
#include <functional>
#include <string>

namespace term_todos {

using ErrorHandler = std::function<void(const std::string& error_message)>;

// Executes a callable inside a try-catch boundary. Catches std::exception and
// unexpected exceptions, forwarding the error message to the provided error
// handler (or a default handler if null). Returns true on success, false on error.
template <typename Func>
bool safe_execute(Func&& func, const ErrorHandler& on_error = nullptr) noexcept {
    try {
        func();
        return true;
    } catch (const std::exception& e) {
        if (on_error) {
            on_error(e.what());
        }
        return false;
    } catch (...) {
        if (on_error) {
            on_error("An unknown error occurred");
        }
        return false;
    }
}

} // namespace term_todos
