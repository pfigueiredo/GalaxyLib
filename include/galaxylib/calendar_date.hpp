#pragma once

namespace galaxy {

/// Gregorian calendar date (month 1–12, day 1–31). Used for orbit time relative to the legacy reference.
struct CalendarDate {
    int year{};
    int month{};
    int day{};
};

/// Reference instant for orbital angle in `SystemBody::update_system_location` (matches C# `UpdateLocation` base).
inline constexpr CalendarDate k_orbit_reference_date{2012, 1, 1};

/// Julian day number (integer) for a Gregorian date (algorithm widely used; e.g. Fliegel–Van Flandern form).
inline int gregorian_to_julian_day_number(int y, int m, int d) {
    const int a = (14 - m) / 12;
    const int yy = y + 4800 - a;
    const int mm = m + 12 * a - 3;
    return d + (153 * mm + 2) / 5 + 365 * yy + yy / 4 - yy / 100 + yy / 400 - 32045;
}

/// Whole calendar days from `from` to `to`. Negative if `to` is before `from`.
inline double days_between(CalendarDate from, CalendarDate to) {
    return static_cast<double>(
        gregorian_to_julian_day_number(to.year, to.month, to.day) -
        gregorian_to_julian_day_number(from.year, from.month, from.day));
}

/// Days from `k_orbit_reference_date` to `d` (same axis as `update_system_location(double)`).
inline double days_since_orbit_reference(CalendarDate d) {
    return days_between(k_orbit_reference_date, d);
}

}  // namespace galaxy
