//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "report.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "accounts.hpp"

using namespace budget;

namespace {

void monthly_report(boost::gregorian::greg_year year){
    auto today = boost::gregorian::day_clock::local_day();

    budget::money max_expenses;
    budget::money max_earnings;
    budget::money max_balance;
    budget::money min_expenses;
    budget::money min_earnings;
    budget::money min_balance;

    auto sm = start_month(year);

    for(unsigned short i = sm + 1; i < today.month() + 1; ++i){
        boost::gregorian::greg_month month = i;

        auto accounts = all_accounts(year, month);

        budget::money total_expenses;
        budget::money total_earnings;
        budget::money total_balance;

        for(auto& account : accounts){
            auto expenses = accumulate_amount_if(all_expenses(), [year,month,account](budget::expense& e){return e.account == account.id && e.date.year() == year && e.date.month() == month;});
            auto earnings = accumulate_amount_if(all_earnings(), [year,month,account](budget::earning& e){return e.account == account.id && e.date.year() == year && e.date.month() == month;});

            total_expenses += expenses;
            total_earnings += earnings;

            auto balance = account.amount;
            balance -= expenses;
            balance += earnings;

            total_balance += balance;
        }

        max_expenses = std::max(max_expenses, total_expenses);
        max_earnings = std::max(max_earnings, total_earnings);
        max_balance = std::max(max_balance, total_balance);
        min_expenses = std::min(min_expenses, total_expenses);
        min_earnings = std::min(min_earnings, total_earnings);
        min_balance = std::min(min_balance, total_balance);
    }
}

} //end of anonymous namespace

void budget::report_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::report_module::handle(const std::vector<std::string>& args){
    auto today = boost::gregorian::day_clock::local_day();

    if(args.size() == 1){
        monthly_report(today.year());
    } else {
        auto& subcommand = args[1];

        if(subcommand == "monthly"){
            monthly_report(today.year());
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}
