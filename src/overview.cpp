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

void budget::handle_overview(const std::vector<std::string>& args){
    if(args.size() == 1){
        month_overview();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "month"){
            month_overview();
        } else if(subcommand == "year"){
            year_overview();
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::month_overview(){
    load_accounts();
    load_expenses();

    std::cout << "Overview of the month" << std::endl << std::endl;

    std::vector<std::string> columns;
    std::unordered_map<std::string, std::size_t> indexes;
    std::vector<std::vector<std::string>> contents;
    std::vector<money> totals;

    for(auto& account : all_accounts()){
        indexes[account.name] = columns.size();
        columns.push_back(account.name);
        totals.push_back({});
    }

    std::vector<std::size_t> current(columns.size(), 0);

    for(auto& expense : all_expenses()){
        std::size_t index = indexes[expense.account];
        std::size_t& row = current[index];

        if(contents.size() <= row){
            contents.emplace_back(columns.size(), "");
        }

        contents[row][index] = expense.name + ":" + to_string(expense.amount);

        totals[index] += expense.amount;

        ++row;
    }

    contents.emplace_back(columns.size(), "");

    std::vector<std::string> total_line;
    for(auto& money : totals){
        total_line.push_back(to_string(money));
    }
    contents.push_back(std::move(total_line));

    display_table(columns, contents);
    std::cout << std::endl;
}

void budget::year_overview(){
    load_accounts();
    load_expenses();

    std::cout << "Overview of the year" << std::endl;

    //TODO
}
