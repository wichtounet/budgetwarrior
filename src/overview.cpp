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
                throw budget_exception("Too many arguments to overview month");
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
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

template<typename T>
T& default_operator(T& t){
    return t;
}

template<typename T, typename J>
void add_recap_line(std::vector<std::vector<std::string>>& contents, const std::string& title, const std::vector<T>& values, J functor){
    std::vector<std::string> total_line;

    total_line.push_back("");
    total_line.push_back(title);
    total_line.push_back(to_string(functor(values.front())));

    for(std::size_t i = 1; i < values.size(); ++i){
        total_line.push_back("");
        total_line.push_back("");
        total_line.push_back(to_string(functor(values[i])));
    }

    contents.push_back(std::move(total_line));
}

template<typename T>
void add_recap_line(std::vector<std::vector<std::string>>& contents, const std::string& title, const std::vector<T>& values){
    return add_recap_line(contents, title, values, [](const T& t){return t;});
}

std::vector<budget::money> compute_total_budget(boost::gregorian::greg_month month, boost::gregorian::greg_year year){
    std::vector<budget::money> total_budgets;

    auto& accounts = all_accounts();

    for(auto& account : accounts){
        budget::money total;

        total += account.amount * month;

        for(auto& expense : all_expenses()){
            if(expense.expense_date.year() == year && expense.expense_date.month() < month){
                total -= expense.amount;
            }
        }

        total_budgets.push_back(total);
    }

    return std::move(total_budgets);
}

void budget::month_overview(boost::gregorian::greg_month month, boost::gregorian::greg_year year){
    load_accounts();
    load_expenses();

    auto& accounts = all_accounts();

    std::cout << "Overview of " << month << " " << year << std::endl << std::endl;

    std::vector<std::string> columns;
    std::unordered_map<std::string, std::size_t> indexes;
    std::vector<std::vector<std::string>> contents;
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

    //Totals
    contents.emplace_back(columns.size() * 3, "");
    add_recap_line(contents, "Total", totals);

    //Budget
    contents.emplace_back(columns.size() * 3, "");
    add_recap_line(contents, "Budget", accounts, [](const budget::account& a){return a.amount;});
    auto total_budgets = compute_total_budget(month, year);
    add_recap_line(contents, "Total Budget", total_budgets);

    //Balances
    contents.emplace_back(columns.size() * 3, "");

    std::vector<budget::money> balances;
    std::vector<budget::money> local_balances;

    for(std::size_t i = 0; i < accounts.size(); ++i){
        balances.push_back(total_budgets[i] - totals[i]);
        local_balances.push_back(accounts[i].amount - totals[i]);
    }

    add_recap_line(contents, "Balance", balances);
    add_recap_line(contents, "Local Balance", local_balances);

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
