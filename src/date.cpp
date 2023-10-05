//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <charconv>

#include "cpp_utils/assert.hpp"

#include "date.hpp"
#include "config.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "data_cache.hpp"
#include "views.hpp"

budget::date budget::local_day(){
    auto tt = time(nullptr);
    auto* timeval = localtime(&tt);

    return {
        static_cast<date_type>(timeval->tm_year + 1900),
        static_cast<date_type>(timeval->tm_mon + 1),
        static_cast<date_type>(timeval->tm_mday)};
}

budget::date budget::date_from_string(std::string_view str){
    if (str.size() != 10) {
        throw date_exception("Invalid size for date_from_string");
    }

    date_type y = 0;
    date_type m = 0;
    date_type d = 0;

    if (auto [p, ec] = std::from_chars(str.data(), str.data() + 4, y); ec != std::errc() || p != str.data() + 4) {
        throw date_exception("Invalid year in date_from_string");
    }

    if (auto [p, ec] = std::from_chars(str.data() + 5, str.data() + 7, m); ec != std::errc() || p != str.data() + 7) {
        throw date_exception("Invalid month in date_from_string");
    }

    if (auto [p, ec] = std::from_chars(str.data() + 8, str.data() + 10, d); ec != std::errc() || p != str.data() + 10) {
        throw date_exception("Invalid day in date_from_string");
    }

    return {y, m, d};
}

std::string budget::date_to_string(const budget::date& date) {
    std::string str(10, '0');

    str[4] = '-';
    str[7] = '-';

    // Convert the year
    if (auto [p, ec] = std::to_chars(str.data(), str.data() + 4, static_cast<date_type>(date.year())); ec != std::errc() || p != str.data() + 4) {
        throw date_exception("Can't convert year to string");
    }

    // Convert the month
    auto* month_ptr = date.month() < budget::month(10) ? str.data() + 6 : str.data() + 5;
    if (auto [p, ec] = std::to_chars(month_ptr, str.data() + 7, static_cast<date_type>(date.month())); ec != std::errc() || p != str.data() + 7) {
        throw date_exception("Can't convert month to string");
    }

    // Convert the month
    auto* day_ptr = date.day() < 10 ? str.data() + 9 : str.data() + 8;
    if (auto [p, ec] = std::to_chars(day_ptr, str.data() + 10, static_cast<date_type>(date.day())); ec != std::errc() || p != str.data() + 10) {
        throw date_exception("Can't convert day to string");
    }

    return str;
}

budget::month budget::start_month(data_cache & cache, budget::year year){
    const budget::month m = min_with_default(cache.expenses() | filter_by_year(year) | to_month, 12);
    return min_with_default(cache.earnings() | filter_by_year(year) | to_month, m);
}

budget::year budget::start_year(data_cache & cache){
    auto today = budget::local_day();
    const budget::year y = min_with_default(cache.expenses() | not_template | to_year, today.year());
    return min_with_default(cache.earnings() | not_template | to_year, y);
}

std::ostream& budget::operator<<(std::ostream& stream, const date& date){
    return stream << date_to_string(date);
}

std::ostream& budget::operator<<(std::ostream& stream, const month& month){
    return stream << month.as_short_string();
}
