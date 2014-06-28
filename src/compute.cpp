//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <utility>

#include "compute.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "accounts.hpp"

budget::status budget::compute_year_status(){
    auto today = budget::local_day();
    return compute_year_status(today.year(), today.month());
}

budget::status budget::compute_year_status(boost::gregorian::greg_year year){
    return compute_year_status(year, 12);
}

budget::status budget::compute_year_status(boost::gregorian::greg_year year, boost::gregorian::greg_month month){
    budget::status status;

    auto sm = start_month(year);

    for(auto& expense : all_expenses()){
        if(expense.date.year() == year && expense.date.month() >= sm && expense.date.month() <= month){
            status.expenses += expense.amount;
        }
    }

    budget::money year_earnings;
    for(auto& earning : all_earnings()){
        if(earning.date.year() == year && earning.date.month() >= sm && earning.date.month() <= month){
            status.earnings += earning.amount;
        }
    }

    for(unsigned short i = sm; i <= month; ++i){
        boost::gregorian::greg_month month = i;

        for(auto& c : all_accounts(year, month)){
            status.budget += c.amount;
        }
    }

    status.balance = status.budget + status.earnings - status.expenses;

    return std::move(status);
}

budget::status budget::compute_month_status(){
    auto today = budget::local_day();
    return compute_month_status(today.year(), today.month());
}

budget::status budget::compute_month_status(boost::gregorian::greg_month month){
    auto today = budget::local_day();
    return compute_month_status(today.year(), month);
}

budget::status budget::compute_month_status(boost::gregorian::greg_year year, boost::gregorian::greg_month month){
    budget::status status;

    for(auto& expense : all_expenses()){
        if(expense.date.year() == year && expense.date.month() == month){
            status.expenses += expense.amount;
        }
    }

    for(auto& earning : all_earnings()){
        if(earning.date.year() == year && earning.date.month() == month){
            status.earnings += earning.amount;
        }
    }

    for(auto& c : all_accounts(year, month)){
        status.budget += c.amount;
    }

    status.balance = status.budget + status.earnings - status.expenses;

    return std::move(status);
}
