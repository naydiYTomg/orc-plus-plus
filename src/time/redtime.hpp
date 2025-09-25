#pragma once
#include <format>
#include <iostream>
#include <ordefs.hpp>
#include <orc_export.hpp>
#include <Windows.h>
#include <ostream>

#include "arithmetic.hpp"
#include "expected.hpp"
#include "winapi.hpp"
using namespace orc::core::defines;
using namespace orc::expected;
using namespace orc::utils::arithmetic;


namespace orc::time {

    enum class ORC_API timezone {
        UTC0,
        UTC1,
        UTC2,
        UTC3,
        UTC4,
        UTC5,
        UTC6,
        UTC7,
        UTC8,
        UTC9,
    };



    ORC_API constexpr i64 SECONDS_PER_DAY = 60 * 60 * 24;
    ORC_API constexpr i64 SECONDS_PER_HOUR = 3600;
    static constexpr auto is_leap(const i32 year) -> bool {
        return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
    }
    static constexpr auto days_in_month(const i32 year, const i32 month) -> u8 {
        switch (month) {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                return 31;
            case 4:
            case 6:
            case 9:
            case 11:
                return 30;
            case 2:
                return is_leap(year) ? 29 : 28;
            default: return 0;
        }
    }
    static constexpr auto leaps_up_to(const i32 y) -> i64 {
        return static_cast<i64>(y) / 4 - static_cast<i64>(y) / 100 + static_cast<i64>(y) / 400;
    }
    static constexpr auto leaps_from_1970(const i32 year) -> i64 {
        return leaps_up_to(year - 1) - leaps_up_to(1969);
    }
    static constexpr auto leaps_to_1970(const i32 year) -> i64 {
        return leaps_up_to(1969) - leaps_up_to(year - 1);
    }

    enum class ORC_API time_error {
        RangeError,
    };

    class ORC_API time {
    public:

        [[nodiscard]] static auto current() -> time {
            FILETIME ft;
            GetSystemTimeAsFileTime(&ft);
            LARGE_INTEGER li;
            li.LowPart = ft.dwLowDateTime;
            li.HighPart = ft.dwHighDateTime;
            return time{core::win::windows_ticks_to_unix_seconds(li.QuadPart)};
        }
        constexpr time() : seconds(0) {}
        constexpr explicit time(const i64 unix) : seconds(unix) {}
        [[nodiscard]] static auto from_exact_date(const i32 year,
                                                const u8 month,
                                                const u8 day,
                                                const u8 hour,
                                                const u8 min,
                                                const u8 sec
        ) -> ::expected<time, time_error> {
            if (month == 0 || month > 12)
                return err(time_error::RangeError);
            u8 month_ = month;
            if (day == 0 || day > days_in_month(year, month))
                return err(time_error::RangeError);
            u8 day_ = day;
            if (hour > 23)
                return err(time_error::RangeError);
            u8 hour_ = hour;
            if (min > 59)
                return err(time_error::RangeError);
            u8 min_ = min;
            if (sec > 59)
                return err(time_error::RangeError);
            u8 sec_ = sec;

            i64 days_before_year;
            if (year >= 1970) {
                const i64 years = year - 1970;
                const i64 leaps = leaps_from_1970(year);
                days_before_year = years * 365 + leaps;
            } else {
                const i64 years = 1970 - year;
                const i64 leaps = leaps_to_1970(year);
                days_before_year = -(years * 365 + leaps);
            }
            i64 days_before_month = 0;
            for (u8 m = 1; m < month_; ++m)
                days_before_month += static_cast<i64>(days_in_month(year, m));
            const i64 day_offset = static_cast<i64>(day_) - 1;
            auto tmp = checked_add(days_before_year, days_before_month);
            if (!tmp.has_value()) return err(time_error::RangeError);
            i64 total_days = tmp.value();
            tmp = checked_add(total_days, day_offset);
            if (!tmp.has_value()) return err(time_error::RangeError);
            total_days = tmp.value();
            tmp = checked_mul(total_days, SECONDS_PER_DAY);
            if (!tmp.has_value()) return err(time_error::RangeError);
            i64 seconds = tmp.value();
            tmp = checked_add(seconds, static_cast<i64>(hour) * 3600);
            if (!tmp.has_value()) return err(time_error::RangeError);
            seconds = tmp.value();
            tmp = checked_add(seconds, static_cast<i64>(min) * 60);
            if (!tmp.has_value()) return err(time_error::RangeError);
            seconds = tmp.value();
            tmp = checked_add(seconds, static_cast<i64>(sec));
            if (!tmp.has_value()) return err(time_error::RangeError);
            seconds = tmp.value();
            return ok(time{seconds});
        }

        [[nodiscard]] constexpr auto raw_value() const noexcept -> i64 { return seconds; }
        constexpr auto convert(const timezone self_timezone, const timezone to_timezone){
            const i32 offset_hours = static_cast<i32>(to_timezone) - static_cast<i32>(self_timezone);
            seconds = seconds + static_cast<i64>(offset_hours) * SECONDS_PER_HOUR;
        }
        [[nodiscard]] constexpr auto convert(const timezone self_timezone, const timezone to_timezone) const noexcept -> time {
            const i32 offset_hours = static_cast<i32>(to_timezone) - static_cast<i32>(self_timezone);
            return time{seconds+static_cast<i64>(offset_hours)*SECONDS_PER_HOUR};
        }
        constexpr auto convert_utc0(const timezone to_timezone) {
            convert(timezone::UTC0, to_timezone);
        }
        [[nodiscard]] constexpr auto convert_utc0(const timezone to_timezone) const noexcept -> time {
            return convert(timezone::UTC0, to_timezone);
        }

        friend auto operator<<(std::ostream& os, const time& t) -> std::ostream& {
            const i64 days_since_epoch = div_euclid(t.seconds, SECONDS_PER_DAY);
            const i64 secs_of_day = rem_euclid(t.seconds, SECONDS_PER_DAY);

            i32 year = 1970;
            i64 day_of_year = days_since_epoch;
            if (day_of_year >= 0) {
                for (;;) {
                    i64 days_in_this_year;
                    if (is_leap(year)) { days_in_this_year = 366; } else { days_in_this_year = 365; }
                    if (day_of_year >= days_in_this_year) {
                        day_of_year -= days_in_this_year;
                        year += 1;
                    } else { break; }
                }
            } else {
                for (;;) {
                    year -= 1;
                    i64 days_in_prev_year;
                    if (is_leap(year)) { days_in_prev_year = 366; } else { days_in_prev_year = 365; }
                    if (day_of_year + days_in_prev_year >= 0) {
                        day_of_year += days_in_prev_year;
                        break;
                    }
                }
            }

            u8 month = 1;
            i64 day_idx = day_of_year;
            for (u8 m = 1; m <= 12; ++m) {
                const i64 dim = days_in_month(year, m);
                if (day_idx < dim) {
                    month = m;
                    break;
                }
                day_idx -= dim;
            }
            const i32 day = day_idx + 1;
            const i32 hour = secs_of_day / 3600;
            const i32 minute = secs_of_day % 3600 / 60;
            const i32 second = secs_of_day % 60;

            std::string month_str;

            switch (month) {
                case 1: month_str = "Jan"; break;
                case 2: month_str = "Feb"; break;
                case 3: month_str = "Mar"; break;
                case 4: month_str = "Apr"; break;
                case 5: month_str = "May"; break;
                case 6: month_str = "Jun"; break;
                case 7: month_str = "Jul"; break;
                case 8: month_str = "Aug"; break;
                case 9: month_str = "Sep"; break;
                case 10: month_str = "Oct"; break;
                case 11: month_str = "Nov"; break;
                case 12: month_str = "Dec"; break;
                default: month_str = "???"; break;
            }

            os << month_str << " " << day << " " << year << ' ' << std::format("{:02}:{:02}:{:02}", hour, minute, second);
            return os;
        }
    private:
        i64 seconds;
    };
    struct ORC_API clockwatch {

        static auto start() {
            for (;;) {
                const auto now = time::current();
                std::cout << now.convert_utc0(timezone::UTC3) << std::flush;
                Sleep(1000);
                std::cout << "\r                      \r" << std::flush;
                if (core::win::is_key_pressed('q')) break;
            }
        }
    };
}