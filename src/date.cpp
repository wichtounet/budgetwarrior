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

budget::date budget::local_day(){
    auto tt = time( NULL );
    auto timeval = localtime( &tt );

    return {
        static_cast<date_type>(timeval->tm_year + 1900),
        static_cast<date_type>(timeval->tm_mon + 1),
        static_cast<date_type>(timeval->tm_mday)};
}

budget::date budget::from_string(std::string_view str){
    if (str.size() != 10) {
        throw date_exception("Invalid size for from_string");
    }

    date_type y;
    date_type m;
    date_type d;

    if (auto [p, ec] = std::from_chars(str.data(), str.data() + 4, y); ec != std::errc() || p != str.data() + 4) {
        throw date_exception("Invalid year in from_string");
    }

    if (auto [p, ec] = std::from_chars(str.data() + 5, str.data() + 7, m); ec != std::errc() || p != str.data() + 7) {
        throw date_exception("Invalid month in from_string");
    }

    if (auto [p, ec] = std::from_chars(str.data() + 8, str.data() + 10, d); ec != std::errc() || p != str.data() + 10) {
        throw date_exception("Invalid day in from_string");
    }

    return {y, m, d};
}

std::string budget::date_to_string(budget::date date){
    std::string str(10, '0');

    str[4] = '-';
    str[7] = '-';

    // Convert the year
    if (auto [p, ec] = std::to_chars(str.data(), str.data() + 4, static_cast<date_type>(date.year())); ec != std::errc() || p != str.data() + 4) {
        throw date_exception("Can't convert year to string");
    }

    // Convert the month
    auto month_ptr = date.month() < 10 ? str.data() + 6 : str.data() + 5;
    if (auto [p, ec] = std::to_chars(month_ptr, str.data() + 7, static_cast<date_type>(date.month())); ec != std::errc() || p != str.data() + 7) {
        throw date_exception("Can't convert month to string");
    }

    // Convert the month
    auto day_ptr = date.day() < 10 ? str.data() + 9 : str.data() + 8;
    if (auto [p, ec] = std::to_chars(day_ptr, str.data() + 10, static_cast<date_type>(date.day())); ec != std::errc() || p != str.data() + 10) {
        throw date_exception("Can't convert day to string");
    }

    return str;
}

unsigned short budget::start_month(budget::year year){
    budget::month m = 12;

    for(auto& expense : all_expenses()){
        if(expense.date.year() == year){
            m = std::min(expense.date.month(), m);
        }
    }

    for(auto& earning : all_earnings()){
        if(earning.date.year() == year){
            m = std::min(earning.date.month(), m);
        }
    }

    return m;
}

unsigned short budget::start_year(){
    auto today = budget::local_day();
    auto y = today.year();

    for(auto& expense : all_expenses()){
        if(expense.date != TEMPLATE_DATE){
            y = std::min(expense.date.year(), y);
        }
    }

    for(auto& earning : all_earnings()){
        if(earning.date != TEMPLATE_DATE){
            y = std::min(earning.date.year(), y);
        }
    }

    return y;
}

std::ostream& budget::operator<<(std::ostream& stream, const date& date){
    return stream << date.year() << "-" << date.month() << "-" << date.day();
}

std::ostream& budget::operator<<(std::ostream& stream, const month& month){
    return stream << month.as_short_string();
}
