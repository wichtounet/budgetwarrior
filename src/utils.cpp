//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "utils.hpp"
#include "budget_exception.hpp"
#include "assert.hpp"
#include "config.hpp"
#include "expenses.hpp"
#include "earnings.hpp"

#include <cstdio>
#include <sys/ioctl.h>
#include <unistd.h>

void budget::not_empty(const std::string& value, const std::string& message){
    if(value.empty()){
        throw budget_exception(message);
    }
}

unsigned short budget::start_month(boost::gregorian::greg_year year){
    auto key = to_string(year) + "_start";
    if(config_contains(key)){
        auto value = to_number<unsigned short>(config_value(key));
        budget_assert(value < 13 && value > 0, "The start month is incorrect (must be in [1,12])");
        return value;
    }

    return 1;
}

unsigned short budget::start_year(){
    auto today = boost::gregorian::day_clock::local_day();
    boost::gregorian::greg_year y = today.year();

    for(auto& expense : all_expenses()){
        y = std::min(expense.date.year(), y);
    }

    for(auto& earning : all_earnings()){
        y = std::min(earning.date.year(), y);
    }

    return y;
}

unsigned short budget::terminal_width(){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

unsigned short budget::terminal_height(){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}
