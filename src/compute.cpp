//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <utility>

#include "config.hpp"
#include "compute.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "accounts.hpp"
#include "incomes.hpp"
#include "data_cache.hpp"

budget::status budget::compute_year_status(data_cache & cache) {
    auto today = budget::local_day();
    return compute_year_status(cache, today.year(), today.month());
}

budget::status budget::compute_year_status(data_cache & cache, year year) {
    return compute_year_status(cache, year, 12);
}

budget::status budget::compute_year_status(data_cache & cache, year year, month month) {
    budget::status status;

    auto sm = start_month(cache, year);

    status.expenses = fold_left_auto(all_expenses_between(cache, year, sm, month) | to_amount);
    status.earnings = fold_left_auto(all_earnings_between(cache, year, sm, month) | to_amount);

    for (unsigned short i = sm; i <= month; ++i) {
        status.budget += fold_left_auto(all_accounts(cache, year, i) | to_amount);
        status.base_income += get_base_income(cache, budget::date(year, i, 1));
    }

    status.balance = status.budget + status.earnings - status.expenses;
    status.income  = status.base_income + status.earnings;
    status.savings = status.income - status.expenses;

    status.taxes = 0;

    if (has_taxes_account()) {
        auto account_id = taxes_account().id;

        status.taxes = fold_left_auto(all_expenses_between(cache, account_id, year, sm, month) | to_amount);
    }

    return status;
}

budget::status budget::compute_month_status(data_cache & cache) {
    auto today = budget::local_day();
    return compute_month_status(cache, today.year(), today.month());
}

budget::status budget::compute_month_status(data_cache & cache, month month) {
    auto today = budget::local_day();
    return compute_month_status(cache, today.year(), month);
}

budget::status budget::compute_month_status(data_cache & cache, year year, month month) {
    budget::status status;

    status.expenses    = fold_left_auto(all_expenses_month(cache, year, month) | to_amount);
    status.earnings    = fold_left_auto(all_earnings_month(cache, year, month) | to_amount);
    status.budget      = fold_left_auto(all_accounts(cache, year, month) | to_amount);
    status.balance     = status.budget + status.earnings - status.expenses;
    status.base_income = get_base_income(cache, budget::date(year, month, 1));
    status.income      = status.base_income + status.earnings;
    status.savings     = status.income - status.expenses;

    status.taxes = 0;

    if (has_taxes_account()) {
        auto account_id = taxes_account().id;

        status.taxes = fold_left_auto(all_expenses_month(cache, account_id, year, month) | to_amount);
    }

    return status;
}

budget::status budget::compute_avg_month_status(data_cache & cache) {
    auto today = budget::local_day();
    return compute_avg_month_status(cache, today.year(), today.month());
}

budget::status budget::compute_avg_month_status(data_cache & cache, month month) {
    auto today = budget::local_day();
    return compute_avg_month_status(cache, today.year(), month);
}

budget::status budget::compute_avg_month_status(data_cache & cache, year year, month month) {
    budget::status avg_status;

    for (budget::month m = 1; m < month; m = m + 1) {
        auto status = compute_month_status(cache, year, m);

        avg_status.expenses += status.expenses;
        avg_status.earnings += status.earnings;
        avg_status.budget += status.budget;
        avg_status.balance += status.balance;
        avg_status.taxes += status.taxes;
        avg_status.savings += status.savings;
    }

    if (month.value > 1) {
        avg_status.expenses /= month.value - 1;
        avg_status.taxes /= month.value - 1;
        avg_status.earnings /= month.value - 1;
        avg_status.budget /= month.value - 1;
        avg_status.balance /= month.value - 1;
        avg_status.savings /= month.value - 1;
    }

    return avg_status;
}
