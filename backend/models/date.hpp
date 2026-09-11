#pragma once

#include <string>

namespace term_todos {

// Small, timezone-free ISO date ("YYYY-MM-DD") utilities. Internally dates are
// serialised with Howard Hinnant's civil-from-days algorithm, which is
// portable and avoids the ambiguity of timegm/localtime across platforms.

// Serial day number for an ISO date (days relative to 1970-01-01). Returns 0
// for the epoch. Throws std::invalid_argument on a malformed date.
int serial_from_iso(const std::string& iso);

// Inverse of serial_from_iso.
std::string iso_from_serial(int serial);

// Today's date in the local timezone as "YYYY-MM-DD".
std::string iso_today();

// Returns the ISO date offset by `delta` days from `iso`.
std::string iso_add_days(const std::string& iso, int delta);

// Weekday with Monday = 0 ... Sunday = 6 (matches the heatmap row layout).
int weekday_monday_based(const std::string& iso);

} // namespace term_todos
