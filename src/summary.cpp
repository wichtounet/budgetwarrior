//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>
#include <unordered_map>
#include <unordered_set>

#include "cpp_utils/assert.hpp"

#include "summary.hpp"
#include "console.hpp"
#include "accounts.hpp"
#include "compute.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "config.hpp"

using namespace budget;

namespace {

void month_overview(budget::month month, budget::year year){
    // First display overview of the accounts

    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(year);

    columns.push_back("Account");
    columns.push_back("Expenses");
    columns.push_back("Earnings");
    columns.push_back("Balance");
    columns.push_back("Local");

    std::unordered_map<std::string, budget::money> account_previous;

    //Fill the table

    budget::money tot_expenses;
    budget::money tot_earnings;
    budget::money tot_balance;
    budget::money tot_local;

    for(unsigned short i = sm; i <= month; ++i){
        budget::month m = i;

        for(auto& account : all_accounts(year, m)){
            auto total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](budget::expense& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
            auto total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](budget::earning& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});

            auto balance = account_previous[account.name] + account.amount - total_expenses + total_earnings;
            auto local_balance = account.amount - total_expenses + total_earnings;

            if(i == month){
                contents.push_back({account.name});

                contents.back().push_back(format_money(total_expenses));
                contents.back().push_back(format_money(total_earnings));
                contents.back().push_back(format_money(balance));
                contents.back().push_back(format_money(local_balance));

                tot_expenses += total_expenses;
                tot_earnings += total_earnings;
                tot_balance += balance;
                tot_expenses += local_balance;
            }

            account_previous[account.name] = balance;
        }
    }

    display_table(columns, contents);
}

void month_overview(budget::month m){
    month_overview(m, local_day().year());
}

void month_overview(){
    month_overview(local_day().month(), local_day().year());
}

} // end of anonymous namespace

constexpr const std::array<std::pair<const char*, const char*>, 1> budget::module_traits<budget::summary_module>::aliases;

void budget::summary_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::summary_module::handle(std::vector<std::string>& args){
    if(all_accounts().empty()){
        throw budget_exception("No accounts defined, you should start by defining some of them");
    }

    if(args.empty() || args.size() == 1){
        month_overview();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "month"){
            auto today = local_day();

            if(args.size() == 2){
                month_overview();
            } else if(args.size() == 3){
                auto m = budget::month(to_number<unsigned short>(args[2]));

                if (m > today.month()){
                    throw budget_exception("Cannot compute the summary of the future");
                }

                month_overview(m);
            } else if(args.size() == 4){
                auto m = budget::month(to_number<unsigned short>(args[2]));
                auto y = budget::year(to_number<unsigned short>(args[3]));

                if (y> today.year()){
                    throw budget_exception("Cannot compute the summary of the future");
                }

                if (y == today.year() && m > today.month()){
                    throw budget_exception("Cannot compute the summary of the future");
                }

                month_overview(m, y);
            } else {
                throw budget_exception("Too many arguments to overview month");
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

