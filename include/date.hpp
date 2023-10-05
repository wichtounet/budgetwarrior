//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <ctime>
#include <limits>
#include <chrono>

#include "cpp_utils/assert.hpp"

#include "utils.hpp"
#include "budget_exception.hpp"

namespace budget {

/* Ideally, this should be mostly rewritten in terms of std::chrono::year_month_day
 * However, this would require many changes since each conversion must be explicit and many operators are missing
 */

using date_type = uint16_t;

struct date_exception : std::exception {
    explicit date_exception(std::string  message) : message_(std::move(message)) {}

    /*!
     * Return the error message.
     * \return The error message.
     */
    const std::string& message() const {
        return message_;
    }

    const char* what() const noexcept override {
        return message_.c_str();
    }

protected:
    std::string message_;
};

struct day {
    date_type value;
    day(date_type value) : value(value) {}

    operator date_type() const { return value; }

    bool is_default() const {
        return value == 0;
    }
};

struct month {
    date_type value;

    month(date_type value) : value(value) {}

    operator date_type() const { return value; }

    month& operator=(date_type new_value){
        this->value = new_value;

        return *this;
    }

    month operator-(date_type remove) const {
        return {static_cast<date_type>(value - remove)};
    }

    month operator+(date_type add) const {
        return {static_cast<date_type>(value + add)};
    }

    std::string as_short_string() const {
        static constexpr const std::array months{"Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

        return months[value-1];
    }

    std::string as_long_string() const {
        static constexpr const std::array months{"January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"};

        return months[value-1];
    }

    bool is_default() const {
        return value == 0;
    }

    bool is_valid() const {
        return value < 13;
    }

    bool is_last() const {
        return value == 12;
    }

    month operator++() {
        ++value;
        return *this;
    }

    friend auto operator<=>(const month & lhs, const month & rhs) = default;
};

struct year {
    date_type value;
    year(date_type value) : value(value) {}
    operator date_type() const { return value; }

    year& operator=(date_type new_value){
        this->value = new_value;

        return *this;
    }

    year operator-(date_type remove) const {
        return {static_cast<date_type>(value - remove)};
    }

    year operator+(date_type add) const {
        return {static_cast<date_type>(value + add)};
    }

    bool is_default() const {
        return value == 0;
    }

    year operator++() {
        ++value;
        return *this;
    }

    friend auto operator<=>(const year & lhs, const year & rhs) = default;
};

struct days {
    date_type value;
    explicit days(date_type value) : value(value) {}
    operator date_type() const { return value; }
};

struct months {
    date_type value;
    explicit months(date_type value) : value(value) {}
    operator date_type() const { return value; }

    months& operator=(date_type new_value){
        this->value = new_value;

        return *this;
    }
};

struct years {
    date_type value;
    explicit years(date_type value) : value(value) {}
    explicit operator date_type() const { return value; }
};

struct date;

std::ostream& operator<<(std::ostream& stream, const month& month);
std::ostream& operator<<(std::ostream& stream, const date& date);

struct date {
    date_type _year;
    date_type _month;
    date_type _day;

    explicit date() = default;

    date(date_type year, date_type month, date_type day) : _year(year), _month(month), _day(day) {
        if(year < 1400){
            throw date_exception(std::format("Year not in the valid range: {}", year));
        }

        if(month == 0 || month > 12){
            throw date_exception(std::format("Invalid month: {}", month));
        }

        if(day == 0 || day > days_month(year, month)){
            throw date_exception(std::format("Invalid day: {}", day));
        }
    }

    // TODO date(year y, month m, day d) : date(y.value, m.value, d.value) {}

    budget::year year() const {
        return budget::year{_year};
    }

    budget::month month() const {
        return budget::month{_month};
    }

    budget::day day() const {
        return budget::day{_day};
    }

    // The number of days of this year until today
    // January 1 is 1
    date_type day_of_year() const {
        date_type result = 0;

        for (date_type m = 1; m < _month; ++m) {
            result += days_month(_year, m);
        }

        result += _day;

        return result;
    }

    // The current week number
    date_type week() const {
        return 1 + day_of_year() / 7;
    }

    date start_of_week() const {
        date r = *this;

        auto w = r.week();
        auto d = r.day_of_year();

        r -= days(d - (w - 1) * 7);

        return r;
    }

    // ISO-8601 Week Number
    date_type iso_week() const {
        const auto doy      = day_of_year();
        const auto dow      = day_of_the_week() - 1;
        const auto dowFirst = date(_year, 1, 1).day_of_the_week() - 1;

        if (dow < dowFirst) {
            return (doy + 6) / 7 + 1;
        }

        return (doy + 6) / 7;
    }

    date iso_start_of_week() const {
        const size_t w = iso_week();

        date r = *this;

        if (w == 1) [[unlikely]] {
            for (size_t i = 0; i < 8; ++i) {
                r -= days(1);

                if (r.iso_week() > w) {
                    break;
                }
            }

            const date r1 = r.iso_start_of_week();
            if (*this - r1 >= 7) {
                return r + days(1);
            }

            return r1;
        }

        for (size_t i = 0; i < 8; ++i) {
            r -= days(1);

            if (r.iso_week() < w) {
                return r + days(1);
            }
        }

        throw budget_exception("Invalid state in iso_start_of_week", true);
    }

    date_type day_of_the_week() const {
        static constexpr const std::array<date_type, 12> t{0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

        // Thanks to leap years, this has to be complicated!

        const date_type y = _year - (_month < 3);
        const date_type dow = (y + y / 4 - y / 100 + y / 400 + t[_month - 1] + _day) % 7;
        return dow ? dow : 7;
    }

    static date_type days_month(date_type year, date_type month){
        static constexpr const std::array<date_type, 12> month_days{31,0,31,30,31,30,31,31,30,31,30,31};

        if(month == 2){
            return is_leap(year) ? 29 : 28;
        }

        return month_days[month - 1];
    }

    static bool is_leap(date_type year){
        return
                ((year % 4 == 0) && year % 100 != 0)
            ||  year % 400 == 0;
    }

    bool is_leap() const {
        return is_leap(_year);
    }

    date start_of_year() const {
        return {_year, 1, 1};
    }

    date end_of_year() const {
        return {_year, 12, days_month(_year, 12)};
    }

    date start_of_month() const {
        return {_year, _month, 1};
    }

    date end_of_month() const {
        return {_year, _month, days_month(_year, _month)};
    }

    date& operator+=(years years){
        if(static_cast<date_type>(years) >= std::numeric_limits<date_type>::max() - _year){
            throw date_exception("Year too high (will overflow)");
        }

        _year += static_cast<date_type>(years);

        return *this;
    }

    date& operator+=(months months){
        // Handle the NOP addition
        if(months == 0){
            return *this;
        }

        // First add several years if necessary
        if (months >= 12) {
            *this += years(months.value / 12);
            months = months.value % 12;
        }

        // Add the remaining months
        _month += months;

        // Update the year if necessary
        if(_month > 12){
            *this += years((_month - 1) / 12);
            _month = _month % 12;
        }

        // Update the day of month, if necessary
        _day = std::min(_day, days_month(_year, _month));

        return *this;
    }

    date& operator+=(days d){
        while(d > 0){
            ++_day;

            if(_day > days_month(_year, _month)){
                _day = 1;
                ++_month;

                if(_month > 12){
                    _month = 1;
                    ++_year;
                }
            }

            d = days(d - 1);
        }

        return *this;
    }

    date& operator-=(years years){
        if(_year < static_cast<date_type>(years)){
            throw date_exception("Year too low");
        }

        _year -= static_cast<date_type>(years);

        return *this;
    }

    date& operator-=(months months){
        // Handle the NOP subtraction
        if(months == 0){
            return *this;
        }

        // First remove several years if necessary
        if (months >= 12) {
            *this -= years(months.value / 12);
            months = months.value % 12;
        }

        if(_month == months){
            *this -= years(1);
            _month = 12;
        } else if(_month < months){
            *this -= years(1);
            _month = 12 - (months - _month);
        } else {
            _month -= months;
        }

        _day = std::min(_day, days_month(_year, _month));

        return *this;
    }

    date& operator-=(days d){
        while(d > 0){
            --_day;
            if(_day == 0){
                --_month;

                if(_month == 0){
                    --_year;
                    _month = 12;
                }

                _day = days_month(_year, _month);
            }

            d = days(d - 1);
        }

        return *this;
    }

    date operator+(years years) const {
        date d(*this);
        d += years;
        return d;
    }

    date operator+(months months) const {
        date d(*this);
        d += months;
        return d;
    }

    date operator+(days days) const {
        date d(*this);
        d += days;
        return d;
    }

    date operator-(years years) const {
        date d(*this);
        d -= years;
        return d;
    }

    date operator-(months months) const {
        date d(*this);
        d -= months;
        return d;
    }

    date operator-(days days) const {
        date d(*this);
        d -= days;
        return d;
    }

    friend auto operator<=>(const date & lhs, const date & rhs) = default;

    int64_t operator-(const date& rhs) const {
        if(*this == rhs){
            return 0;
        }

        if(rhs > *this){
            return -(rhs - *this);
        }

        auto x = *this;
        int64_t d = 0;

        while(x != rhs){
            x -= days(1);
            ++d;
        }

        return d;
    }
};

date local_day();

date date_from_string(std::string_view str);

std::string date_to_string(const date& date);

template<>
inline std::string to_string(budget::date date){
    return date_to_string(date);
}

struct data_cache;

budget::year start_year(data_cache & cache);
budget::month start_month(data_cache & cache, budget::year year);

} //end of namespace budget

namespace std {

template <>
struct hash<budget::day> {
    std::size_t operator()(budget::day d) const noexcept {
        const std::hash<budget::date_type> hasher;
        return hasher(d.value);
    }
};

template <>
struct hash<budget::month> {
    std::size_t operator()(budget::month d) const noexcept {
        const std::hash<budget::date_type> hasher;
        return hasher(d.value);
    }
};

template <>
struct hash<budget::year> {
    std::size_t operator()(budget::year d) const noexcept {
        const std::hash<budget::date_type> hasher;
        return hasher(d.value);
    }
};

template <>
struct hash<budget::date> {
    std::size_t operator()(budget::date d) const noexcept {
        const std::hash<budget::date_type> hasher;
        auto seed = hasher(d._day);
        seed ^= hasher(d._month) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= hasher(d._year) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        return seed;
    }
};

} // end of namespace std
