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
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "config.hpp"
#include "assert.hpp"

using namespace budget;

namespace {

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
    if(m.positive()){
        return "::green" + budget::to_string(m);
    } else if(m.negative()){
        return "::red" + budget::to_string(m);
    } else if(m.zero()){
        return budget::to_string(m);
    } else {
        return budget::to_string(m);
    }
}

std::vector<budget::money> compute_total_budget(boost::gregorian::greg_month month, boost::gregorian::greg_year year){
    std::vector<budget::money> total_budgets;

    auto accounts = all_accounts(year, month);

    auto sm = start_month(year);

    for(auto& account : accounts){
        auto total = account.amount * (month - sm + 1);

        total -= accumulate_amount_if(all_expenses(), [year,month,account,sm](budget::expense& e){return e.account == account.id && e.date.year()  == year && e.date.month() >= sm && e.date.month() < month;});
        total += accumulate_amount_if(all_earnings(), [year,month,account,sm](budget::earning& e){return e.account == account.id && e.date.year()  == year && e.date.month() >= sm && e.date.month() < month;});

        total_budgets.push_back(total);
    }

    return std::move(total_budgets);
}

template<typename T>
void add_values_column(boost::gregorian::greg_month month, boost::gregorian::greg_year year, const std::string& title, std::vector<std::vector<std::string>>& contents, std::unordered_map<std::size_t, std::size_t>& indexes, std::size_t columns, std::vector<T>& values, std::vector<budget::money>& total){
    std::vector<std::size_t> current(columns, contents.size());

    std::vector<T> sorted_values = values;
    std::sort(sorted_values.begin(), sorted_values.end(), [](const T& a, const T& b){ return a.date < b.date; });

    for(auto& expense : sorted_values){
        if(expense.date.year() == year && expense.date.month() == month){
            std::size_t index = indexes[expense.account];
            std::size_t& row = current[index];

            if(contents.size() <= row){
                contents.emplace_back(columns * 3, "");
            }

            contents[row][index * 3] = to_string(expense.date.day());
            contents[row][index * 3 + 1] = expense.name;
            contents[row][index * 3 + 2] = to_string(expense.amount);

            total[index] += expense.amount;

            ++row;
        }
    }

    //Totals of expenses
    contents.emplace_back(columns * 3, "");
    add_recap_line(contents, title, total);
}

void month_overview(boost::gregorian::greg_month month, boost::gregorian::greg_year year){
    auto accounts = all_accounts(year, month);

    std::cout << "Overview of " << month << " " << year << std::endl << std::endl;

    std::vector<std::string> columns;
    std::unordered_map<std::size_t, std::size_t> indexes;
    std::vector<std::vector<std::string>> contents;
    std::vector<money> total_expenses(accounts.size(), budget::money());
    std::vector<money> total_earnings(accounts.size(), budget::money());

    for(auto& account : accounts){
        indexes[account.id] = columns.size();
        columns.push_back(account.name);
    }

    //Expenses
    add_values_column(month, year, "Expenses", contents, indexes, columns.size(), all_expenses(), total_expenses);

    //Earnings
    contents.emplace_back(columns.size() * 3, "");
    add_values_column(month, year, "Earnings", contents, indexes, columns.size(), all_earnings(), total_earnings);

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
        balances.push_back(total_budgets[i] - total_expenses[i] + total_earnings[i]);
        local_balances.push_back(accounts[i].amount - total_expenses[i] + total_earnings[i]);
    }

    add_recap_line(contents, "Balance", balances, [](const budget::money& m){ return format_money(m);});
    add_recap_line(contents, "Local Balance", local_balances, [](const budget::money& m){ return format_money(m);});

    display_table(columns, contents, 3);

    std::cout << std::endl;

    auto total_all_expenses = std::accumulate(total_expenses.begin(), total_expenses.end(), budget::money());
    std::cout << std::string(accounts.size() * 10, ' ')         << "Total expenses: " << total_all_expenses << std::endl;

    auto total_all_earnings = std::accumulate(total_earnings.begin(), total_earnings.end(), budget::money());
    std::cout << std::string(accounts.size() * 10, ' ')         << "Total earnings: " << total_all_earnings << std::endl;

    auto total_balance = std::accumulate(balances.begin(), balances.end(), budget::money());
    std::cout << std::string(accounts.size() * 10 + 7, ' ')     <<        "Balance: " << format(format_money(total_balance)) << format_code(0,0,7) << std::endl;

    auto total_local_balance = std::accumulate(local_balances.begin(), local_balances.end(), budget::money());
    std::cout << std::string(accounts.size() * 10 + 1, ' ')     <<  "Local Balance: " << format(format_money(total_local_balance)) << format_code(0,0,7) << std::endl;
}

void month_overview(boost::gregorian::greg_month month){
    auto today = boost::gregorian::day_clock::local_day();

    month_overview(month, today.year());
}

void month_overview(){
    auto today = boost::gregorian::day_clock::local_day();

    month_overview(today.month(), today.year());
}

void display_local_balance(boost::gregorian::greg_year year);
void display_balance(boost::gregorian::greg_year year);
void display_expenses(boost::gregorian::greg_year year);
void display_earnings(boost::gregorian::greg_year year);

bool invalid_accounts(boost::gregorian::greg_year year){
    auto sm = start_month(year);

    std::vector<budget::account> previous = all_accounts(year, sm);;

    for(unsigned short i = sm + 1; i < 13; ++i){
        boost::gregorian::greg_month month = i;

        auto current_accounts = all_accounts(year, month);

        if(current_accounts.size() != previous.size()){
            return true;
        }

        for(auto& c : current_accounts){
            bool found = false;

            for(auto& p : previous){
                if(p.name == c.name){
                    found = true;
                    break;
                }
            }

            if(!found){
                return true;
            }
        }

        previous = std::move(current_accounts);
    }

    return false;
}

void year_overview(boost::gregorian::greg_year year){
    if(invalid_accounts(year)){
        throw budget::budget_exception("The accounts of the different months have different names, impossible to generate the year overview. ");
    }

    std::cout << "Overview of " << year << std::endl << std::endl;

    display_local_balance(year);
    std::cout << std::endl;

    display_balance(year);
    std::cout << std::endl;

    display_expenses(year);
    std::cout << std::endl;

    display_earnings(year);
    std::cout << std::endl;
}

void year_overview(){
    auto today = boost::gregorian::day_clock::local_day();

    year_overview(today.year());
}

void add_month_columns(std::vector<std::string>& columns, boost::gregorian::greg_month sm){
    for(unsigned short i = sm; i < 13; ++i){
        boost::gregorian::greg_month m = i;

        columns.emplace_back(m.as_long_string());
    }
}

int get_current_months(boost::gregorian::greg_year year){
    auto sm = start_month(year);
    auto current_months = 12 - sm + 1;

    auto today = boost::gregorian::day_clock::local_day();

    if(today.year() == year){
        current_months = today.month() - sm + 1;
    }

    return current_months;
}

template<bool Mean = false, bool CMean = false>
inline void generate_total_line(std::vector<std::vector<std::string>>& contents, std::vector<budget::money>& totals, boost::gregorian::greg_year year, boost::gregorian::greg_month sm){
    std::vector<std::string> last_row;
    last_row.push_back("Total");

    auto current_months = get_current_months(year);

    budget::money total_total;
    budget::money current_total;
    for(unsigned short i = sm; i < 13; ++i){
        auto total = totals[i - 1];

        last_row.push_back(format_money(total));

        total_total += total;

        boost::gregorian::greg_month m = i;
        if(m < sm + current_months){
            current_total += total;
        }
    }

    if(Mean){
        last_row.push_back(format_money(total_total));
        last_row.push_back(format_money(total_total / (12 - sm + 1)));
    }

    if(CMean){
        last_row.push_back(format_money(current_total));
        last_row.push_back(format_money(current_total / current_months));
    }

    contents.emplace_back(std::move(last_row));
}

void display_local_balance(boost::gregorian::greg_year year){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(year);
    auto months = 12 - sm + 1;
    auto current_months = get_current_months(year);

    columns.push_back("Local Balance");
    add_month_columns(columns, sm);
    columns.push_back("Total");
    columns.push_back("Mean");
    columns.push_back("C. Total");
    columns.push_back("C. Mean");

    std::vector<budget::money> totals(12, budget::money());

    std::unordered_map<std::string, std::size_t> row_mapping;
    std::unordered_map<std::string, budget::money> account_totals;
    std::unordered_map<std::string, budget::money> account_current_totals;

    //Prepare the rows

    for(auto& account : all_accounts(year, sm)){
        row_mapping[account.name] = contents.size();

        contents.push_back({account.name});
    }

    //Fill the table

    for(unsigned short i = sm; i < 13; ++i){
        boost::gregorian::greg_month m = i;

        for(auto& account : all_accounts(year, m)){
            auto total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](budget::expense& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
            auto total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](budget::earning& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});

            auto month_total = account.amount - total_expenses + total_earnings;

            contents[row_mapping[account.name]].push_back(format_money(month_total));

            account_totals[account.name] += month_total;

            totals[i - 1] += month_total;

            if(m < sm + current_months){
                account_current_totals[account.name] += month_total;
            }
        }
    }

    //Generate total and mean columns for each account

    for(auto& account : all_accounts(year, sm)){
        contents[row_mapping[account.name]].push_back(format_money(account_totals[account.name]));
        contents[row_mapping[account.name]].push_back(format_money(account_totals[account.name] / months));
        contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name]));
        contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name] / current_months));
    }

    //Generate the total final line

    generate_total_line<true, true>(contents, totals, year, sm);

    display_table(columns, contents);
}

void display_balance(boost::gregorian::greg_year year){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(year);

    columns.push_back("Balance");
    add_month_columns(columns, sm);

    std::vector<budget::money> totals(12, budget::money());

    std::unordered_map<std::string, std::size_t> row_mapping;
    std::unordered_map<std::string, std::vector<budget::money>> account_previous;

    //Prepare the rows

    for(auto& account : all_accounts(year, sm)){
        row_mapping[account.name] = contents.size();

        contents.push_back({account.name});
        account_previous[account.name] = std::vector<budget::money>(13, budget::money());
    }

    //Fill the table

    for(unsigned short i = sm; i < 13; ++i){
        boost::gregorian::greg_month m = i;

        for(auto& account : all_accounts(year, m)){
            auto total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](budget::expense& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
            auto total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](budget::earning& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});

            auto month_total = account_previous[account.name][i - 1] + account.amount - total_expenses + total_earnings;
            account_previous[account.name][i] = month_total;

            totals[i - 1] += month_total;

            contents[row_mapping[account.name]].push_back(format_money(month_total));
        }
    }

    //Generate the final total line

    generate_total_line(contents, totals, year, sm);

    display_table(columns, contents);
}

template<typename T>
void display_values(boost::gregorian::greg_year year, const std::string& title, const std::vector<T>& values){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(year);
    auto months = 12 - sm + 1;
    auto current_months = get_current_months(year);

    columns.push_back(title);
    add_month_columns(columns, sm);
    columns.push_back("Total");
    columns.push_back("Mean");
    columns.push_back("C. Total");
    columns.push_back("C. Mean");

    std::unordered_map<std::string, std::size_t> row_mapping;
    std::unordered_map<std::string, budget::money> account_totals;;
    std::unordered_map<std::string, budget::money> account_current_totals;;
    std::vector<budget::money> totals(13, budget::money());

    //Prepare the rows

    for(auto& account : all_accounts(year, sm)){
        row_mapping[account.name] = contents.size();

        contents.push_back({account.name});
    }

    //Fill the table

    for(unsigned short j = sm; j < 13; ++j){
        boost::gregorian::greg_month m = j;

        for(auto& account : all_accounts(year, m)){
            budget::money month_total;

            for(auto& value : values){
                if(value.account == account.id && value.date.year() == year && value.date.month() == m){
                    month_total += value.amount;
                }
            }

            contents[row_mapping[account.name]].push_back(to_string(month_total));

            account_totals[account.name] += month_total;
            totals[j-1] += month_total;

            if(m < sm + current_months){
                account_current_totals[account.name] += month_total;
            }
        }
    }

    //Generate total and mean columns for each account

    for(auto& account : all_accounts(year, sm)){
        contents[row_mapping[account.name]].push_back(to_string(account_totals[account.name]));
        contents[row_mapping[account.name]].push_back(to_string(account_totals[account.name] / months));
        contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name]));
        contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name] / current_months));
    }

    //Generate the final total line

    generate_total_line<true, true>(contents, totals, year, sm);

    display_table(columns, contents);
}

void display_expenses(boost::gregorian::greg_year year){
    display_values(year, "Expenses", all_expenses());
}

void display_earnings(boost::gregorian::greg_year year){
    display_values(year, "Earnings", all_earnings());
}

} // end of anonymous namespace

void budget::overview_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

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
