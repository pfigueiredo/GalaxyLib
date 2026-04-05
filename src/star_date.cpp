#include "galaxylib/star_date.hpp"

#include "galaxylib/calendar_date.hpp"

#include <cstdint>
#include <cstdio>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace galaxy {
namespace {

bool is_leap_year(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int days_in_month(int y, int m) {
    static constexpr int k[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m < 1 || m > 12) {
        return 31;
    }
    int d = k[m - 1];
    if (m == 2 && is_leap_year(y)) {
        d = 29;
    }
    return d;
}

void add_gregorian_years(int& y, int& m, int& d, int delta_years) {
    y += delta_years;
    const int dim = days_in_month(y, m);
    if (d > dim) {
        d = dim;
    }
}

int day_of_year(int y, int m, int d) {
    static constexpr int k_cum[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    if (m < 1 || m > 12) {
        return d;
    }
    int n = k_cum[m - 1] + d;
    if (m > 2 && is_leap_year(y)) {
        ++n;
    }
    return n;
}

/// Inverse of the same JDN convention as `gregorian_to_julian_day_number` in `calendar_date.hpp` (tested vs 1970 / 3216).
void gregorian_from_julian_day_number(int jdn, int& y, int& m, int& d) {
    int J = jdn + 32044;
    const int g = J / 146097;
    const int dg = J % 146097;
    const int c = (dg / 36524 + 1) * 3 / 4;
    const int dc = dg - c * 36524;
    const int b = dc / 1461;
    const int db = dc % 1461;
    const int a = (db / 365 + 1) * 3 / 4;
    const int da = db - a * 365;
    int Y = g * 400 + c * 100 + b * 4 + a;
    int M = (da * 5 + 308) / 153 - 2;
    int D = da - (M + 4) * 153 / 5 + 122;
    Y = Y - 4800 + (M + 2) / 12;
    M = (M + 2) % 12 + 1;
    D = D + 1;
    y = Y;
    m = M;
    d = D;
}

std::int64_t utc_civil_to_unix_seconds(int y, int m, int d, int hh, int mm, int ss) {
    const int j0 = gregorian_to_julian_day_number(1970, 1, 1);
    const int j = gregorian_to_julian_day_number(y, m, d);
    return static_cast<std::int64_t>(j - j0) * 86400LL + hh * 3600LL + mm * 60LL + ss;
}

void unix_seconds_to_utc_civil(std::int64_t sec, int& y, int& m, int& d, int& hh, int& mm, int& ss) {
    std::int64_t day = sec / 86400;
    std::int64_t rem = sec % 86400;
    if (rem < 0) {
        --day;
        rem += 86400;
    }
    const int jdn = gregorian_to_julian_day_number(1970, 1, 1) + static_cast<int>(day);
    gregorian_from_julian_day_number(jdn, y, m, d);
    hh = static_cast<int>(rem / 3600);
    mm = static_cast<int>((rem % 3600) / 60);
    ss = static_cast<int>(rem % 60);
}

std::int64_t time_point_to_unix_seconds(StarDate::time_point tp) {
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

StarDate::time_point unix_seconds_to_time_point(std::int64_t sec) {
    return StarDate::clock::time_point(std::chrono::seconds(sec));
}

}  // namespace

StarDate StarDate::from_earth_utc(int year, int month, int day, int hour, int minute, int second) {
    return from_earth(unix_seconds_to_time_point(utc_civil_to_unix_seconds(year, month, day, hour, minute, second)));
}

StarDate StarDate::from_earth(time_point earth_instant) {
    const std::int64_t sec_in = time_point_to_unix_seconds(earth_instant);
    int y = 0;
    int m = 0;
    int d = 0;
    int hh = 0;
    int mi = 0;
    int ss = 0;
    unix_seconds_to_utc_civil(sec_in, y, m, d, hh, mi, ss);
    add_gregorian_years(y, m, d, 1204);
    const std::int64_t sec_out = utc_civil_to_unix_seconds(y, m, d, hh, mi, ss);
    return StarDate(unix_seconds_to_time_point(sec_out));
}

double StarDate::s_date() const {
    const std::int64_t sec = time_point_to_unix_seconds(tp_);
    int y = 0;
    int m = 0;
    int d = 0;
    int hh = 0;
    int mi = 0;
    int ss = 0;
    unix_seconds_to_utc_civil(sec, y, m, d, hh, mi, ss);
    const int doy = day_of_year(y, m, d);
    return static_cast<double>(y) + static_cast<double>(doy) / 365.25;
}

std::string StarDate::to_string() const {
    const std::int64_t sec = time_point_to_unix_seconds(tp_);
    int y = 0;
    int m = 0;
    int d = 0;
    int hh = 0;
    int mi = 0;
    int ss = 0;
    unix_seconds_to_utc_civil(sec, y, m, d, hh, mi, ss);

    std::ostringstream sd;
    sd << std::fixed << std::setprecision(4) << s_date();

    char cal[40];
    std::snprintf(cal, sizeof(cal), "%04d-%02d-%02d %02d:%02d:%02d", y, m, d, hh, mi, ss);
    return sd.str() + " EGMT " + cal;
}

}  // namespace galaxy
