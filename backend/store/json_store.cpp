#include "backend/store/json_store.hpp"

#include "backend/controllers/app_state.hpp"
#include "backend/models/date.hpp"
#include "backend/store/json_codecs.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>

namespace term_todos {

namespace {

void rebase_history_to_today(AppState& state) {
    int newest = 0;
    bool any = false;
    for (const auto& habit : state.habits) {
        for (const auto& [date, done] : habit.history) {
            if (!done) continue;
            try {
                newest = std::max(newest, serial_from_iso(date));
                any = true;
            } catch (const std::invalid_argument&) {
            }
        }
    }
    if (!any) return;
    const int delta = serial_from_iso(iso_today()) - newest;
    if (delta == 0) return;

    for (auto& habit : state.habits) {
        std::map<std::string, bool> shifted;
        for (const auto& [date, done] : habit.history) {
            try {
                shifted[iso_add_days(date, delta)] = done;
            } catch (const std::invalid_argument&) {
                shifted[date] = done;
            }
        }
        habit.history = std::move(shifted);
    }
}

std::string parent_dir(const std::string& path) {
    const auto pos = path.find_last_of('/');
    return pos == std::string::npos ? std::string(".") : path.substr(0, pos);
}

std::string csv_field(const std::string& value) {
    if (value.find_first_of(",\"\n") == std::string::npos) return value;
    std::string result = "\"";
    for (char c : value) result += c == '"' ? "\"\"" : std::string(1, c);
    return result + "\"";
}

} // namespace

bool load_state(const std::string& path, AppState& out, bool rebase) {
    std::ifstream file(path);
    if (!file) return false;
    std::stringstream buffer;
    buffer << file.rdbuf();

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(buffer.str());
    } catch (const nlohmann::json::parse_error&) {
        return false;
    }
    if (!root.is_object()) return false;

    AppState next;
    if (!decode_state_records(root, next)) return false;
    next.clamp_selection();
    next.data_path = path;
    next.dirty = false;
    if (rebase) rebase_history_to_today(next);
    out = std::move(next);
    return true;
}

bool save_state(const std::string& path, const AppState& state) {
    const nlohmann::json root = encode_state_records(state);
    const std::string tmp = parent_dir(path) + "/.term-todos.tmp";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out << root.dump(2);
        if (!out) return false;
        out.flush();
    }
    return std::rename(tmp.c_str(), path.c_str()) == 0;
}

bool export_habits_csv(const std::string& path, const AppState& state, int days) {
    if (days <= 0) return false;
    const int today = serial_from_iso(iso_today());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return false;

    out << "habit";
    for (int i = days - 1; i >= 0; --i) out << "," << iso_from_serial(today - i);
    out << "\n";

    for (const auto& habit : state.habits) {
        out << csv_field(habit.name);
        for (int i = days - 1; i >= 0; --i) {
            const std::string date = iso_from_serial(today - i);
            out << "," << (habit.done_on(date) ? "1" : "");
        }
        out << "\n";
    }
    return static_cast<bool>(out);
}

} // namespace term_todos
