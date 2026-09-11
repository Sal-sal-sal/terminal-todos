#include "backend/models/date.hpp"

#include <cstdio>
#include <ctime>
#include <stdexcept>

namespace term_todos {
namespace {

// Howard Hinnant's days_from_civil. Returns days since 1970-01-01 (can be
// negative for earlier dates) and works for any valid proleptic Gregorian date.
int days_from_civil(int y, unsigned m, unsigned d) {
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);          // [0, 399]
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; // [0, 365]
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;          // [0, 146096]
    return era * 146097 + static_cast<int>(doe) - 719468;
}

// Inverse of days_from_civil.
void civil_from_days(int z, int& y, unsigned& m, unsigned& d) {
    z += 719468;
    const int era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);         // [0, 146096]
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365; // [0, 399]
    y = static_cast<int>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);        // [0, 365]
    const unsigned mp = (5 * doy + 2) / 153;                             // [0, 11]
    d = doy - (153 * mp + 2) / 5 + 1;                                    // [1, 31]
    m = mp + (mp < 10 ? 3 : -2);                                         // [1, 12]
}

void parse_iso(const std::string& iso, int& y, unsigned& m, unsigned& d) {
    if (iso.size() != 10 || iso[4] != '-' || iso[7] != '-') {
        throw std::invalid_argument("invalid ISO date: " + iso);
    }
    y = std::stoi(iso.substr(0, 4));
    m = static_cast<unsigned>(std::stoi(iso.substr(5, 2)));
    d = static_cast<unsigned>(std::stoi(iso.substr(8, 2)));
}

} // namespace

int serial_from_iso(const std::string& iso) {
    int y;
    unsigned m, d;
    parse_iso(iso, y, m, d);
    return days_from_civil(y, m, d);
}

std::string iso_from_serial(int serial) {
    int y;
    unsigned m, d;
    civil_from_days(serial, y, m, d);
    char buf[11];
    std::snprintf(buf, sizeof(buf), "%04d-%02u-%02u", y, m, d);
    return std::string(buf);
}

std::string iso_today() {
    std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    char buf[11];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
                  local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
    return std::string(buf);
}

std::string iso_add_days(const std::string& iso, int delta) {
    return iso_from_serial(serial_from_iso(iso) + delta);
}

int weekday_monday_based(const std::string& iso) {
    // 1970-01-01 was a Thursday. (serial + 4) % 7 yields 0=Sunday..6=Saturday;
    // shift so Monday=0.
    const int sunday_based = (serial_from_iso(iso) + 4) % 7;
    return (sunday_based + 6) % 7;
}

} // namespace term_todos
