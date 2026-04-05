#pragma once

#include <chrono>
#include <string>

namespace galaxy {

/// Matches `GalaxyE.Galaxy.StarDate`: in-universe calendar with `SDate` and `GetStarDate` (+1204 years on Earth time).
class StarDate {
public:
    using clock = std::chrono::system_clock;
    using time_point = clock::time_point;

    /// Stores `tp` as-is (C# `new StarDate(dateTime)`).
    explicit StarDate(time_point tp) : tp_(tp) {}

    /// C# `StarDate.GetStarDate(date)`: same instant shifted forward by 1204 Gregorian years (`.AddYears(1204)`).
    [[nodiscard]] static StarDate from_earth(time_point earth_instant);

    /// Earth calendar in UTC, then `from_earth` (convenient for tests and tools).
    [[nodiscard]] static StarDate from_earth_utc(int year, int month, int day, int hour = 0, int minute = 0,
                                                  int second = 0);

    /// `GetStarDate(DateTime.Now)` — uses the system clock (same idea as C# `StarDate.Now`).
    [[nodiscard]] static StarDate now() { return from_earth(clock::now()); }

    /// C# `SDate`: `Date.Year + Date.DayOfYear / 365.25` on the **stored** calendar (internal `DateTime`).
    [[nodiscard]] double s_date() const;

    /// C# `ToString`: `"{s_date:0.0000} EGMT {short date} {short time}"` — uses fixed `YYYY-MM-DD` and `HH:MM:SS` (culture-neutral).
    [[nodiscard]] std::string to_string() const;

    [[nodiscard]] time_point internal_time_point() const { return tp_; }

private:
    time_point tp_{};
};

}  // namespace galaxy
