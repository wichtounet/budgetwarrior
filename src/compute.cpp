//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <utility>

#include "compute.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "accounts.hpp"

budget::status budget::compute_year_status() {
    auto today = budget::local_day();
    return compute_year_status(today.year(), today.month());
}

budget::status budget::compute_year_status(year year) {
    return compute_year_status(year, 12);
}

budget::status budget::compute_year_status(year year, month month) {
    budget::status status;

    auto sm = start_month(year);

    status.expenses = accumulate_amount(all_expenses_between(year, sm, month));
    status.earnings = accumulate_amount(all_earnings_between(year, sm, month));

    for (unsigned short i = sm; i <= month; ++i) {
        status.budget += accumulate_amount(all_accounts(year, i));
    }

    status.balance = status.budget + status.earnings - status.expenses;

    return status;
}

budget::status budget::compute_month_status() {
    auto today = budget::local_day();
    return compute_month_status(today.year(), today.month());
}

budget::status budget::compute_month_status(month month) {
    auto today = budget::local_day();
    return compute_month_status(today.year(), month);
}

budget::status budget::compute_month_status(year year, month month) {
    budget::status status;

    status.expenses = accumulate_amount(all_expenses_month(year, month));
    status.earnings = accumulate_amount(all_earnings_month(year, month));
    status.budget   = accumulate_amount(all_accounts(year, month));
    status.balance  = status.budget + status.earnings - status.expenses;

    return status;
}

budget::status budget::compute_avg_month_status() {
    auto today = budget::local_day();
    return compute_avg_month_status(today.year(), today.month());
}

budget::status budget::compute_avg_month_status(month month) {
    auto today = budget::local_day();
    return compute_avg_month_status(today.year(), month);
}

budget::status budget::compute_avg_month_status(year year, month month) {
    budget::status avg_status;

    for (budget::month m = 1; m < month; m = m + 1) {
        auto status = compute_month_status(year, m);

        avg_status.expenses += status.expenses;
        avg_status.earnings += status.earnings;
        avg_status.budget += status.budget;
        avg_status.balance += status.balance;
    }

    if (month.value > 1) {
        avg_status.expenses /= month.value - 1;
        avg_status.earnings /= month.value - 1;
        avg_status.budget /= month.value - 1;
        avg_status.balance /= month.value - 1;
    }

    return std::move(avg_status);
}
