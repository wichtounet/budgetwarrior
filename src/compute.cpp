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

budget::status budget::compute_year_status(year year){
    return compute_year_status(year, 12);
}

budget::status budget::compute_year_status(year year, month month){
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
        budget::month month = i;

        for(auto& c : all_accounts(year, month)){
            status.budget += c.amount;
        }
    }

    status.balance = status.budget + status.earnings - status.expenses;

    return status;
}

budget::status budget::compute_month_status(){
    auto today = budget::local_day();
    return compute_month_status(today.year(), today.month());
}

budget::status budget::compute_month_status(month month){
    auto today = budget::local_day();
    return compute_month_status(today.year(), month);
}

budget::status budget::compute_month_status(year year, month month){
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

budget::status budget::compute_avg_month_status(){
    auto today = budget::local_day();
    return compute_avg_month_status(today.year(), today.month());
}

budget::status budget::compute_avg_month_status(month month){
    auto today = budget::local_day();
    return compute_avg_month_status(today.year(), month);
}

budget::status budget::compute_avg_month_status(year year, month month){
    budget::status avg_status;

    for(budget::month m = 1; m < month; m = m + 1){
        auto status = compute_month_status(year, m);

        avg_status.expenses += status.expenses;
        avg_status.earnings += status.earnings;
        avg_status.budget += status.budget;
        avg_status.balance += status.balance;
    }

    avg_status.expenses /= month.value;
    avg_status.earnings /= month.value;
    avg_status.budget /= month.value;
    avg_status.balance /= month.value;

    return std::move(avg_status);
}
