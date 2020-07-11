//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "date.hpp"
#include "budget_exception.hpp"
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

budget::date budget::from_string(const std::string& str){
    auto y = year(to_number<unsigned short>(str.substr(0, 4)));
    auto m = month(to_number<unsigned short>(str.substr(5, 2)));
    auto d = day(to_number<unsigned short>(str.substr(8, 2)));

    return {y, m, d};
}

budget::date budget::from_iso_string(const std::string& str){
    auto y = year(to_number<unsigned short>(str.substr(0, 4)));
    auto m = month(to_number<unsigned short>(str.substr(4, 2)));
    auto d = day(to_number<unsigned short>(str.substr(6, 2)));

    return {y, m, d};
}

std::string budget::date_to_string(budget::date date){
    return std::to_string(date.year())
        + "-" + (date.month() < 10 ? "0" : "") + std::to_string(date.month())
        + "-" + (date.day() < 10 ? "0" : "") + std::to_string(date.day());
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
