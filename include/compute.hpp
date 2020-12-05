//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "money.hpp"
#include "date.hpp"

namespace budget {

struct status {
    budget::money expenses;
    budget::money taxes;
    budget::money earnings;
    budget::money budget;
    budget::money balance;
    budget::money base_income;
    budget::money income;
    budget::money savings;

    status add_expense(budget::money expense){
        auto new_status = *this;

        new_status.expenses += expense;
        new_status.balance -= expense;

        return new_status;
    }

    status add_earning(budget::money earning){
        auto new_status = *this;

        new_status.earnings += earning;
        new_status.balance += earning;

        return new_status;
    }

    budget::money expenses_no_taxes() const {
        return expenses - taxes;
    }

    double tax_rate() const {
        return 100.0 * (taxes / income);
    }

    double savings_rate() const {
        if (savings.dollars() > 0) {
            return 100.0 * (savings / income);
        } else {
            return 0.0;
        }
    }

    double savings_rate_after_tax() const {
        auto income_after_tax   = income - taxes;
        auto expenses_after_tax = expenses - taxes;
        auto savings_after_tax  = income_after_tax - expenses_after_tax;

        if (savings_after_tax.dollars() > 0) {
            return 100.0 * (savings_after_tax / income_after_tax);
        } else {
            return 0.0;
        }
    }
};

struct data_cache;

status compute_year_status(data_cache & cache);
status compute_year_status(data_cache & cache, budget::year year);
status compute_year_status(data_cache & cache, budget::year year, budget::month last);

status compute_month_status(data_cache & cache);
status compute_month_status(data_cache & cache, budget::month year);
status compute_month_status(data_cache & cache, budget::year year, budget::month month);

status compute_avg_month_status(data_cache & cache);
status compute_avg_month_status(data_cache & cache, budget::month year);
status compute_avg_month_status(data_cache & cache, budget::year year, budget::month month);

} //end of namespace budget
