//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <map>
#include <unordered_map>
#include <unordered_set>

#include "cpp_utils/assert.hpp"

#include "predict.hpp"
#include "overview.hpp"
#include "console.hpp"
#include "accounts.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "config.hpp"
#include "writer.hpp"
#include "data_cache.hpp"

using namespace budget;

namespace {

void predict_overview(){
    auto today = budget::local_day();

    data_cache cache;

    auto expenses = cache.expenses();
    auto earnings = cache.earnings();

    auto accounts = current_accounts(cache);

    std::map<std::string, size_t, std::less<>> account_mappings;

    std::vector<double> expense_multipliers(accounts.size(), 100.0);
    std::vector<double> earning_multipliers(accounts.size(), 100.0);

    std::cout << "Multipliers for expenses" << std::endl;

    size_t i = 0;
    for(auto& account : accounts){
        account_mappings[account.name] = i;

        std::cout << "   ";
        edit_double(expense_multipliers[i], account.name);

        ++i;
    }

    for(auto& expense : expenses){
        if(account_mappings.contains(get_account(expense.account).name)){
            expense.amount *= (expense_multipliers[account_mappings[get_account(expense.account).name]] / 100.0);
        }
    }

    for(auto& earning : earnings){
        if(account_mappings.contains(get_account(earning.account).name)){
            earning.amount *= (earning_multipliers[account_mappings[get_account(earning.account).name]] / 100.0);
        }
    }

    if (today.month() < 12) {
        auto prev_year = today.year() - 1;
        for (budget::month m = today.month() + 1; m <= 12; m = m + 1) {
            for (auto& expense : expenses | filter_by_date(prev_year, m)) {
                expense.date = {today.year(), expense.date.month(), expense.date.day()};
            }

            for (auto& earning : earnings | filter_by_date(prev_year, m)) {
                earning.date = {today.year(), earning.date.month(), earning.date.day()};
            }
        }
    }

    console_writer w(std::cout);

    display_local_balance(w, today.year(), false, true);
    display_balance(w, today.year(), true);
    display_expenses(w, today.year(), false, true);
    display_earnings(w, today.year(), false, true);
}

} // end of anonymous namespace

void budget::predict_module::load() const {
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::predict_module::handle(std::vector<std::string>& args) const {
    if(no_accounts()){
        throw budget_exception("No accounts defined, you should start by defining some of them");
    }

    if(args.empty() || args.size() == 1){
        predict_overview();
    } else {
        throw budget_exception("Too many arguments");
    }
}
