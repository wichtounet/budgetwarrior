//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef COMPUTE_H
#define COMPUTE_H

#include "money.hpp"
#include "date.hpp"

namespace budget {

struct status {
    budget::money expenses;
    budget::money earnings;
    budget::money budget;
    budget::money balance;

    status add_expense(budget::money expense){
        auto new_status = *this;

        new_status.expenses += expense;
        new_status.balance -= expense;

        return std::move(new_status);
    }

    status add_earning(budget::money earning){
        auto new_status = *this;

        new_status.earnings += earning;
        new_status.balance += earning;

        return std::move(new_status);
    }
};

status compute_year_status();
status compute_year_status(budget::year year);
status compute_year_status(budget::year year, budget::month last);

status compute_month_status();
status compute_month_status(budget::month year);
status compute_month_status(budget::year year, budget::month month);

status compute_avg_month_status();
status compute_avg_month_status(budget::month year);
status compute_avg_month_status(budget::year year, budget::month month);

} //end of namespace budget

#endif
