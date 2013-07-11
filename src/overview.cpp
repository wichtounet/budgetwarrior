//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>

#include "overview.hpp"
#include "console.hpp"
#include "accounts.hpp"
#include "expenses.hpp"
#include "budget_exception.hpp"

using namespace budget;

void budget::handle_overview(const std::vector<std::wstring>& args){
    if(args.size() == 1){
        month_overview();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "month"){
            if(args.size() == 2){
                month_overview();
            } else if(args.size() == 3){
                month_overview(boost::gregorian::greg_month(to_number<unsigned short>(args[2])));
            } else if(args.size() == 4){
                month_overview(
                    boost::gregorian::greg_month(to_number<unsigned short>(args[2])),
                    boost::gregorian::greg_year(to_number<unsigned short>(args[3])));
            } else {
                throw budget_exception("Too many arguments to overview month");
            }
        } else if(subcommand == "year"){
            if(args.size() == 2){
                year_overview();
            } else if(args.size() == 3){
                year_overview(boost::gregorian::greg_year(to_number<unsigned short>(args[2])));
            } else {
            }
        } else {
            throw budget_exception(L"Invalid subcommand \"" + subcommand + L"\"");
        }
    }
}

void budget::month_overview(){
    date today = boost::gregorian::day_clock::local_day();

    month_overview(today.month(), today.year());
}

void budget::month_overview(boost::gregorian::greg_month month){
    date today = boost::gregorian::day_clock::local_day();

    month_overview(month, today.year());
}

void budget::month_overview(boost::gregorian::greg_month month, boost::gregorian::greg_year year){
    load_accounts();
    load_expenses();

    auto& accounts = all_accounts();

    std::cout << "Overview of " << month << " " << year << std::endl << std::endl;

    std::vector<std::wstring> columns;
    std::unordered_map<std::wstring, std::size_t> indexes;
    std::vector<std::vector<std::wstring>> contents;
    std::vector<money> totals;

    for(auto& account : accounts){
        indexes[account.name] = columns.size();
        columns.push_back(account.name);
        totals.push_back({});
    }

    std::vector<std::size_t> current(columns.size(), 0);

    for(auto& expense : all_expenses()){
        if(expense.expense_date.year() == year && expense.expense_date.month() == month){
            std::size_t index = indexes[expense.account];
            std::size_t& row = current[index];

            if(contents.size() <= row){
                contents.emplace_back(columns.size() * 3, "");
            }

            contents[row][index * 3] = to_string(expense.expense_date.day());
            contents[row][index * 3 + 1] = expense.name;
            contents[row][index * 3 + 2] = to_string(expense.amount);

            totals[index] += expense.amount;

            ++row;
        }
    }

    //Empty line before totals
    contents.emplace_back(columns.size() * 3, "");

    std::vector<std::string> total_line;

    total_line.push_back("");
    total_line.push_back("Total");
    total_line.push_back(to_string(totals.front()));

    for(std::size_t i = 1; i < totals.size(); ++i){
        total_line.push_back("");
        total_line.push_back("");
        total_line.push_back(to_string(totals[i]));
    }

    contents.push_back(std::move(total_line));

    //Empty line before budget
    std::vector<std::string> budget_line;

    budget_line.push_back("");
    budget_line.push_back("Budget");
    budget_line.push_back(to_string(accounts.front().amount));

    for(std::size_t i = 1; i < accounts.size(); ++i){
        budget_line.push_back("");
        budget_line.push_back("");
        budget_line.push_back(to_string(accounts[i].amount));
    }

    contents.push_back(std::move(budget_line));

    std::vector<std::string> total_budget_line;

    total_budget_line.push_back("");
    total_budget_line.push_back("Budget total");
    total_budget_line.push_back(to_string(accounts.front().amount));

    for(std::size_t i = 1; i < accounts.size(); ++i){
        total_budget_line.push_back("");
        total_budget_line.push_back("");
        total_budget_line.push_back("TODO");
    }

    contents.push_back(std::move(total_budget_line));

    //Empty line before remainings
    contents.emplace_back(columns.size() * 3, "");

    std::vector<std::string> balance_line;

    balance_line.push_back("");
    balance_line.push_back("Balance");
    balance_line.push_back("TODO");

    for(std::size_t i = 1; i < accounts.size(); ++i){
        balance_line.push_back("");
        balance_line.push_back("");
        balance_line.push_back("TODO");
    }

    contents.push_back(std::move(balance_line));

    std::vector<std::string> local_balance_line;

    local_balance_line.push_back("");
    local_balance_line.push_back("Local Balance");
    local_balance_line.push_back("TODO");

    for(std::size_t i = 1; i < accounts.size(); ++i){
        local_balance_line.push_back("");
        local_balance_line.push_back("");
        local_balance_line.push_back("TODO");
    }

    contents.push_back(std::move(local_balance_line));

    display_table(columns, contents, 3);

    std::cout << std::endl;

    money total_expenses;
    for(auto& total : totals){
        total_expenses += total;
    }

    std::cout << std::string(accounts.size() * 10, ' ') << "Total expenses: " << total_expenses << std::endl;
    std::cout << std::string(accounts.size() * 10 + 7, ' ') <<        "Balance: " << "TODO" << std::endl;
}

void budget::year_overview(){
    date today = boost::gregorian::day_clock::local_day();

    year_overview(today.year());
}

void budget::year_overview(boost::gregorian::greg_year year){
    load_accounts();
    load_expenses();

    std::cout << "Overview of " << year << std::endl;

    //TODO
}
