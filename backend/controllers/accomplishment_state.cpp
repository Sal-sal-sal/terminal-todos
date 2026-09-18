#include "backend/controllers/app_state.hpp"
#include <algorithm>

namespace term_todos {
namespace {
std::string trimmed(const std::string& value) {
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return {};
    return value.substr(start, value.find_last_not_of(" \t\r\n") - start + 1);
}
}

void AppState::navigate_accomplishment(int delta) {
    const int count = static_cast<int>(accomplishments.size());
    selected_accomplishment = count ? ((selected_accomplishment + delta) % count + count) % count : 0;
}

void AppState::add_accomplishment(const std::string& title) {
    const auto value = trimmed(title);
    if (value.empty()) return;
    accomplishments.push_back({value, false});
    selected_accomplishment = static_cast<int>(accomplishments.size()) - 1;
    dirty = true;
}

void AppState::rename_accomplishment(const std::string& title) {
    const auto value = trimmed(title);
    if (accomplishments.empty() || value.empty()) return;
    clamp_selection();
    accomplishments[selected_accomplishment].title = value;
    dirty = true;
}

void AppState::toggle_accomplishment() {
    if (accomplishments.empty()) return;
    clamp_selection();
    auto& item = accomplishments[selected_accomplishment];
    item.done = !item.done;
    dirty = true;
}

void AppState::delete_accomplishment() {
    if (accomplishments.empty()) return;
    clamp_selection();
    accomplishments.erase(accomplishments.begin() + selected_accomplishment);
    clamp_selection();
    dirty = true;
}
}
