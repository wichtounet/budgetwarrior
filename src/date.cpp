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
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};

    return {
        static_cast<date_type>(static_cast<int>(ymd.year())),
        static_cast<date_type>(static_cast<unsigned>(ymd.month())),
        static_cast<date_type>(static_cast<unsigned>(ymd.day()))};
}

budget::date budget::date_from_string(std::string_view str){
    if (str.size() != 10) {
        throw date_exception(std::format("Invalid size for date_from_string while parsing {}", str));
    }

    date_type y = 0;
    date_type m = 0;
    date_type d = 0;

    if (auto [p, ec] = std::from_chars(str.data(), str.data() + 4, y); ec != std::errc() || p != str.data() + 4) {
        throw date_exception(std::format("Invalid year in date_from_string while parsing {}", str));
    }

    if (auto [p, ec] = std::from_chars(str.data() + 5, str.data() + 7, m); ec != std::errc() || p != str.data() + 7) {
        throw date_exception(std::format("Invalid month in date_from_string while parsing {}", str));
    }

    if (auto [p, ec] = std::from_chars(str.data() + 8, str.data() + 10, d); ec != std::errc() || p != str.data() + 10) {
        throw date_exception(std::format("Invalid day in date_from_string while parsing {}", str));
    }

    return {y, m, d};
}

budget::date budget::dmy_date_from_string(std::string_view str){
    if (str.size() != 10) {
        throw date_exception(std::format("Invalid size for dmy_date_from_string while parsing {}", str));
    }

    date_type y = 0;
    date_type m = 0;
    date_type d = 0;

    if (auto [p, ec] = std::from_chars(str.data(), str.data() + 2, d); ec != std::errc() || p != str.data() + 2) {
        throw date_exception(std::format("Invalid day in dmy_date_from_string while parsing {}", str));
    }

    if (auto [p, ec] = std::from_chars(str.data() + 3, str.data() + 5, m); ec != std::errc() || p != str.data() + 5) {
        throw date_exception(std::format("Invalid month in dmy_date_from_string while parsing {}", str));
    }

    if (auto [p, ec] = std::from_chars(str.data() + 6, str.data() + 10, y); ec != std::errc() || p != str.data() + 10) {
        throw date_exception(std::format("Invalid year in dmy_date_from_string while parsing {}", str));
    }

    return {y, m, d};
}

budget::year budget::year_from_string(std::string_view str){
    return budget::year{to_number<date_type>(str)};
}

budget::month budget::month_from_string(std::string_view str){
    return budget::month{to_number<date_type>(str)};
}

budget::day budget::day_from_string(std::string_view str){
    return budget::day{to_number<date_type>(str)};
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
    auto* day_ptr = static_cast<date_type>(date.day()) < 10 ? str.data() + 9 : str.data() + 8;
    if (auto [p, ec] = std::to_chars(day_ptr, str.data() + 10, static_cast<date_type>(date.day())); ec != std::errc() || p != str.data() + 10) {
        throw date_exception("Can't convert day to string");
    }

    return str;
}

budget::month budget::start_month(data_cache & cache, budget::year year){
    const budget::month m = min_with_default(cache.expenses() | persistent | filter_by_year(year) | to_month, budget::month(12));
    return min_with_default(cache.earnings() | filter_by_year(year) | to_month, m);
}

budget::year budget::start_year(data_cache & cache){
    auto today = budget::local_day();
    const budget::year y = min_with_default(cache.expenses() | persistent | not_template | to_year, today.year());
    return min_with_default(cache.earnings() | not_template | to_year, y);
}

std::ostream& budget::operator<<(std::ostream& stream, const date& date){
    return stream << date_to_string(date);
}

std::ostream& budget::operator<<(std::ostream& stream, const year& year){
    return stream << year.value;
}

std::ostream& budget::operator<<(std::ostream& stream, const month& month){
    return stream << month.as_short_string();
}

std::ostream& budget::operator<<(std::ostream& stream, const day& day){
    return stream << day.value;
}
