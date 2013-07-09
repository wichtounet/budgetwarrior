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

        if(subcommand == L"month"){
            month_overview();
        } else if(subcommand == L"year"){
            year_overview();
        } else {
            throw budget_exception(L"Invalid subcommand \"" + subcommand + L"\"");
        }
    }
}

void budget::month_overview(){
    load_accounts();
    load_expenses();

    auto& accounts = all_accounts();

    auto today = boost::gregorian::day_clock::local_day();

    std::wcout << L"Overview of " << today.month() << " " << today.year() << std::endl << std::endl;

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
        std::size_t index = indexes[expense.account];
        std::size_t& row = current[index];

        if(contents.size() <= row){
            contents.emplace_back(columns.size() * 3, L"");
        }

        contents[row][index * 3] = to_string(expense.expense_date.day());
        contents[row][index * 3 + 1] = expense.name;
        contents[row][index * 3 + 2] = to_string(expense.amount);

        totals[index] += expense.amount;

        ++row;
    }

    //Empty line before totals
    contents.emplace_back(columns.size() * 3, L"");

    std::vector<std::wstring> total_line;

    total_line.push_back(L"");
    total_line.push_back(L"Total");
    total_line.push_back(to_string(totals.front()));

    for(std::size_t i = 1; i < totals.size(); ++i){
        total_line.push_back(L"");
        total_line.push_back(L"");
        total_line.push_back(to_string(totals[i]));
    }

    contents.push_back(std::move(total_line));

    //Empty line before budget
    contents.emplace_back(columns.size() * 3, L"");

    std::vector<std::wstring> budget_line;

    budget_line.push_back(L"");
    budget_line.push_back(L"Budget");
    budget_line.push_back(to_string(accounts.front().amount));

    for(std::size_t i = 1; i < accounts.size(); ++i){
        budget_line.push_back(L"");
        budget_line.push_back(L"");
        budget_line.push_back(to_string(accounts[i].amount));
    }

    contents.push_back(std::move(budget_line));

    std::vector<std::wstring> total_budget_line;

    total_budget_line.push_back(L"");
    total_budget_line.push_back(L"Budget total");
    total_budget_line.push_back(L"TODO");

    for(std::size_t i = 1; i < accounts.size(); ++i){
        total_budget_line.push_back(L"");
        total_budget_line.push_back(L"");
        total_budget_line.push_back(L"TODO");
    }

    contents.push_back(std::move(total_budget_line));

    //Empty line before balances
    contents.emplace_back(columns.size() * 3, L"");

    std::vector<std::wstring> balance_line;

    balance_line.push_back(L"");
    balance_line.push_back(L"Balance");
    balance_line.push_back(L"TODO");

    for(std::size_t i = 1; i < accounts.size(); ++i){
        balance_line.push_back(L"");
        balance_line.push_back(L"");
        balance_line.push_back(L"TODO");
    }

    contents.push_back(std::move(balance_line));

    std::vector<std::wstring> local_balance_line;

    local_balance_line.push_back(L"");
    local_balance_line.push_back(L"Local Balance");
    local_balance_line.push_back(L"TODO");

    for(std::size_t i = 1; i < accounts.size(); ++i){
        local_balance_line.push_back(L"");
        local_balance_line.push_back(L"");
        local_balance_line.push_back(L"TODO");
    }

    contents.push_back(std::move(local_balance_line));

    display_table(columns, contents, 3);
    std::wcout << std::endl;

    money total_expenses;
    for(auto& total : totals){
        total_expenses += total;
    }

    std::wcout << std::wstring(accounts.size() * 10, ' ') << L"Total expenses: " << total_expenses << std::endl;
    std::wcout << std::wstring(accounts.size() * 10 + 7, ' ') <<        L"Balance: " << L"TODO" << std::endl;
}

void budget::year_overview(){
    load_accounts();
    load_expenses();

    std::wcout << L"Overview of the year" << std::endl;

    //TODO
}
