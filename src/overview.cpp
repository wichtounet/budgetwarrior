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
#include "config.hpp"
#include "assert.hpp"

using namespace budget;

namespace {

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

std::string format_money(const budget::money& m){
    if(m.dollars > 0){
        return "::green" + budget::to_string(m);
    } else if(m.dollars < 0){
        return "::red" + budget::to_string(m);
    } else {
        return budget::to_string(m);
    }
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

void month_overview(boost::gregorian::greg_month month, boost::gregorian::greg_year year){
    load_accounts();
    load_expenses();

    auto& accounts = all_accounts();

    std::cout << "Overview of " << month << " " << year << std::endl << std::endl;

    std::vector<std::string> columns;
    std::unordered_map<std::size_t, std::size_t> indexes;
    std::vector<std::vector<std::string>> contents;
    std::vector<money> totals;

    for(auto& account : accounts){
        indexes[account.id] = columns.size();
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
    add_recap_line(contents, "Budget", accounts, [](const budget::account& a){return format_money(a.amount);});
    auto total_budgets = compute_total_budget(month, year);
    add_recap_line(contents, "Total Budget", total_budgets, [](const budget::money& m){ return format_money(m);});

    //Balances
    contents.emplace_back(columns.size() * 3, "");

    std::vector<budget::money> balances;
    std::vector<budget::money> local_balances;

    for(std::size_t i = 0; i < accounts.size(); ++i){
        balances.push_back(total_budgets[i] - totals[i]);
        local_balances.push_back(accounts[i].amount - totals[i]);
    }

    add_recap_line(contents, "Balance", balances, [](const budget::money& m){ return format_money(m);});
    add_recap_line(contents, "Local Balance", local_balances, [](const budget::money& m){ return format_money(m);});

    display_table(columns, contents, 3);

    std::cout << std::endl;

    auto total_expenses = std::accumulate(totals.begin(), totals.end(), budget::money());
    std::cout << std::string(accounts.size() * 10, ' ')         << "Total expenses: " << total_expenses << std::endl;

    auto total_balance = std::accumulate(balances.begin(), balances.end(), budget::money());
    std::cout << std::string(accounts.size() * 10 + 7, ' ')     <<        "Balance: " << format(format_money(total_balance)) << format_code(0,0,7) << std::endl;

    auto total_local_balance = std::accumulate(local_balances.begin(), local_balances.end(), budget::money());
    std::cout << std::string(accounts.size() * 10 + 1, ' ')     <<  "Local Balance: " << format(format_money(total_local_balance)) << format_code(0,0,7) << std::endl;
}

void month_overview(boost::gregorian::greg_month month){
    date today = boost::gregorian::day_clock::local_day();

    month_overview(month, today.year());
}

void month_overview(){
    date today = boost::gregorian::day_clock::local_day();

    month_overview(today.month(), today.year());
}

void display_local_balance(boost::gregorian::greg_year year);
void display_balance(boost::gregorian::greg_year year);
void display_expenses(boost::gregorian::greg_year year);

void year_overview(boost::gregorian::greg_year year){
    load_accounts();
    load_expenses();

    std::cout << "Overview of " << year << std::endl << std::endl;

    display_local_balance(year);
    std::cout << std::endl;

    display_balance(year);
    std::cout << std::endl;

    display_expenses(year);
    std::cout << std::endl;
}

unsigned short start_month(boost::gregorian::greg_year year){
    auto key = to_string(year) + "_start";
    if(config_contains(key)){
        auto value = to_number<unsigned short>(config_value(key));
        budget_assert(value < 13 && value > 0, "The start month is incorrect (must be in [1,12])");
        return value;
    }

    return 1;
}

void display_local_balance(boost::gregorian::greg_year year){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    columns.push_back("Local Balance");

    auto sm = start_month(year);
    auto months = 12 - sm + 1;

    for(unsigned short i = sm; i < 13; ++i){
        boost::gregorian::greg_month m = i;

        columns.emplace_back(m.as_long_string());
    }

    columns.push_back("Total");
    columns.push_back("Mean");

    auto& accounts = all_accounts();
    auto& expenses = all_expenses();

    std::vector<budget::money> totals(12, budget::money());

    for(auto& account : accounts){
        std::vector<std::string> row;

        row.push_back(account.name);

        budget::money total;

        for(unsigned short i = sm; i < 13; ++i){
            boost::gregorian::greg_month m = i;

            budget::money month_total;

            for(auto& expense : expenses){
                if(expense.account == account.id && expense.expense_date.year() == year && expense.expense_date.month() == m){
                   month_total += expense.amount;
                }
            }

            month_total = account.amount - month_total;

            row.push_back(format_money(month_total));

            total += month_total;

            totals[i - 1] += month_total;
        }

        row.push_back(format_money(total));
        row.push_back(format_money(total / months));

        contents.emplace_back(std::move(row));
    }

    std::vector<std::string> last_row;
    last_row.push_back("Total");

    budget::money total_total;
    for(unsigned short i = sm; i < 13; ++i){
        auto total = totals[i - 1];

        last_row.push_back(format_money(total));

        total_total += total;
    }

    last_row.push_back(format_money(total_total));
    last_row.push_back(format_money(total_total / months));

    contents.emplace_back(std::move(last_row));

    display_table(columns, contents);
}

void display_balance(boost::gregorian::greg_year year){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    columns.push_back("Balance");

    auto sm = start_month(year);

    for(unsigned short i = sm; i < 13; ++i){
        boost::gregorian::greg_month m = i;

        columns.emplace_back(m.as_long_string());
    }

    auto& accounts = all_accounts();
    auto& expenses = all_expenses();

    std::vector<budget::money> totals(12, budget::money());

    for(auto& account : accounts){
        std::vector<std::string> row;

        row.push_back(account.name);

        budget::money total;
        std::vector<budget::money> previous(13, budget::money());

        for(unsigned short i = sm; i < 13; ++i){
            boost::gregorian::greg_month m = i;

            budget::money month_total;

            for(auto& expense : expenses){
                if(expense.account == account.id && expense.expense_date.year() == year && expense.expense_date.month() == m){
                   month_total += expense.amount;
                }
            }

            month_total = previous[i - 1] + account.amount - month_total;
            previous[i] = month_total;

            totals[i - 1] += month_total;

            row.push_back(format_money(month_total));
        }

        contents.emplace_back(std::move(row));
    }

    std::vector<std::string> last_row;
    last_row.push_back("Total");

    budget::money total_total;
    for(unsigned short i = sm; i < 13; ++i){
        auto total = totals[i - 1];
        last_row.push_back(format_money(total));
    }

    contents.emplace_back(std::move(last_row));

    display_table(columns, contents);
}

void display_expenses(boost::gregorian::greg_year year){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(year);
    auto months = 12 - sm + 1;

    columns.push_back("Expenses");

    for(unsigned short i = sm; i < 13; ++i){
        boost::gregorian::greg_month m = i;

        columns.emplace_back(m.as_long_string());
    }

    columns.push_back("Total");
    columns.push_back("Mean");

    auto& accounts = all_accounts();
    auto& expenses = all_expenses();

    std::vector<budget::money> totals(13, budget::money());

    for(std::size_t i = 0; i < accounts.size(); ++i){
        auto& account = accounts[i];

        std::vector<std::string> row;

        row.push_back(account.name);

        budget::money total;

        for(unsigned short j = sm; j < 13; ++j){
            boost::gregorian::greg_month m = j;

            budget::money month_total;

            for(auto& expense : expenses){
                if(expense.account == account.id && expense.expense_date.year() == year && expense.expense_date.month() == m){
                   month_total += expense.amount;
                }
            }

            row.push_back(to_string(month_total));

            total += month_total;
            totals[j-1] += month_total;
        }

        row.push_back(to_string(total));
        row.push_back(to_string(total / months));

        contents.emplace_back(std::move(row));
    }

    std::vector<std::string> last_row;
    last_row.push_back("Total");

    budget::money total_total;
    for(unsigned short j = sm; j < 13; ++j){
        auto total = totals[j-1];

        last_row.push_back(to_string(total));

        total_total += total;
    }

    last_row.push_back(to_string(total_total));
    last_row.push_back(to_string(total_total / months));

    contents.emplace_back(std::move(last_row));

    display_table(columns, contents);
}

void year_overview(){
    date today = boost::gregorian::day_clock::local_day();

    year_overview(today.year());
}

} // end of anonymous namespace

void budget::overview_module::handle(const std::vector<std::string>& args){
    if(args.empty() || args.size() == 1){
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
