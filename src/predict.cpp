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

using namespace budget;

namespace {

void predict_overview(){
    auto today = budget::local_day();

    auto expenses = all_expenses();
    auto earnings = all_earnings();

    auto accounts = current_accounts();

    std::map<std::string, size_t> account_mappings;

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
        if(account_mappings.count(get_account(expense.account).name)){
            expense.amount *= (expense_multipliers[account_mappings[get_account(expense.account).name]] / 100.0);
        }
    }

    for(auto& earning : earnings){
        if(account_mappings.count(get_account(earning.account).name)){
            earning.amount *= (earning_multipliers[account_mappings[get_account(earning.account).name]] / 100.0);
        }
    }

    if(today.month() < 12){
        auto prev_year = today.year() - 1;
        for(budget::month m = today.month() + 1; m <= 12; m = m + 1){
            for(auto& expense : expenses){
                if(expense.date.month() == m && expense.date.year() == prev_year){
                    expense.date = {today.year(), expense.date.month(), expense.date.day()};
                }
            }

            for(auto& earning : earnings){
                if(earning.date.month() == m && earning.date.year() == prev_year){
                    earning.date = {today.year(), earning.date.month(), earning.date.day()};
                }
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

void budget::predict_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::predict_module::handle(std::vector<std::string>& args){
    if(no_accounts()){
        throw budget_exception("No accounts defined, you should start by defining some of them");
    }

    if(args.empty() || args.size() == 1){
        predict_overview();
    } else {
        throw budget_exception("Too many arguments");
    }
}
