#include "backend/controllers/app_state.hpp"
#include "backend/models/date.hpp"
#include <algorithm>

namespace term_todos {
namespace {
std::string trimmed(const std::string& value) {
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return {};
    return value.substr(start, value.find_last_not_of(" \t\r\n") - start + 1);
}
}

bool AppState::Accomplishment::archived_on(const std::string& today) const {
    return done && (completed_on.empty() || completed_on < today);
}

std::vector<int> AppState::accomplishment_order(const std::string& today) const {
    std::vector<int> order;
    order.reserve(accomplishments.size());
    for (int i = 0; i < static_cast<int>(accomplishments.size()); ++i) {
        if (!accomplishments[i].archived_on(today)) order.push_back(i);
    }
    for (int i = 0; i < static_cast<int>(accomplishments.size()); ++i) {
        if (accomplishments[i].archived_on(today)) order.push_back(i);
    }
    return order;
}

void AppState::navigate_accomplishment(int delta) {
    const auto order = accomplishment_order(iso_today());
    const int count = static_cast<int>(order.size());
    if (!count) {
        selected_accomplishment = 0;
        return;
    }
    const auto found = std::find(order.begin(), order.end(), selected_accomplishment);
    const int position = found == order.end() ? 0 : static_cast<int>(found - order.begin());
    selected_accomplishment = order[((position + delta) % count + count) % count];
}

void AppState::add_accomplishment(const std::string& title) {
    const auto value = trimmed(title);
    if (value.empty()) return;
    accomplishments.push_back({value, false, {}});
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
    item.completed_on = item.done ? iso_today() : "";
    if (item.done) {
        celebrating_accomplishment = selected_accomplishment;
        celebration_started = std::chrono::steady_clock::now();
    } else {
        celebrating_accomplishment = -1;
    }
    dirty = true;
}

void AppState::delete_accomplishment() {
    if (accomplishments.empty()) return;
    clamp_selection();
    const auto order = accomplishment_order(iso_today());
    const auto found = std::find(order.begin(), order.end(), selected_accomplishment);
    const int position = static_cast<int>(found - order.begin());
    const int erased = selected_accomplishment;
    accomplishments.erase(accomplishments.begin() + erased);
    if (celebrating_accomplishment == erased) celebrating_accomplishment = -1;
    else if (celebrating_accomplishment > erased) --celebrating_accomplishment;
    const auto next = accomplishment_order(iso_today());
    selected_accomplishment = next.empty() ? 0 : next[std::min(position, static_cast<int>(next.size()) - 1)];
    dirty = true;
}
}
