//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "date.hpp"
#include "budget_exception.hpp"
#include "assert.hpp"
#include "config.hpp"
#include "expenses.hpp"
#include "earnings.hpp"

budget::date budget::local_day(){
    auto tt = time( NULL );
    auto timeval = localtime( &tt );

    return {timeval->tm_year + 1900, timeval->tm_mon + 1, timeval->tm_mday};
}

unsigned short budget::start_month(budget::year year){
    auto key = to_string(year) + "_start";
    if(config_contains(key)){
        auto value = to_number<unsigned short>(config_value(key));
        budget_assert(value < 13 && value > 0, "The start month is incorrect (must be in [1,12])");
        return value;
    }

    return 1;
}

unsigned short budget::start_year(){
    auto today = budget::local_day();
    auto y = today.year();

    for(auto& expense : all_expenses()){
        y = std::min(expense.date.year(), y);
    }

    for(auto& earning : all_earnings()){
        y = std::min(earning.date.year(), y);
    }

    return y;
}
