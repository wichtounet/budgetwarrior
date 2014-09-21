//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <unordered_map>
#include <unordered_set>

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

bool invalid_accounts(budget::year year){
    auto sm = start_month(year);

    std::vector<budget::account> previous = all_accounts(year, sm);;

    for(unsigned short i = sm + 1; i < 13; ++i){
        budget::month month = i;

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

std::vector<budget::money> compute_total_budget(budget::month month, budget::year year){
    std::unordered_map<std::string, budget::money> tmp;

    for(budget::year y = start_year(); y <= year; y = y + 1){
        budget::month m = start_month(y);
        while(true){
            if(y == year && m >= month){
                break;
            }

            for(auto& account : all_accounts(y, m)){
                tmp[account.name] += account.amount;

                tmp[account.name] -= accumulate_amount_if(all_expenses(), [y,m,account](budget::expense& e){
                    return e.account == account.id && e.date.year() == y && e.date.month() == m;
                    });

                tmp[account.name] += accumulate_amount_if(all_earnings(), [y,m,account](budget::earning& e){
                    return e.account == account.id && e.date.year() == y && e.date.month() == m;
                    });
            }

            if(y != year && m == 12){
                break;
            }

            m = m + 1;
        }
    }

    std::vector<budget::money> total_budgets;

    for(auto& account : all_accounts(year, month)){
        tmp[account.name] += account.amount;

        total_budgets.push_back(tmp[account.name]);
    }

    return std::move(total_budgets);
}

template<typename T>
void add_values_column(budget::month month, budget::year year, const std::string& title, std::vector<std::vector<std::string>>& contents, std::unordered_map<std::string, std::size_t>& indexes, std::size_t columns, std::vector<T>& values, std::vector<budget::money>& total){
    std::vector<std::size_t> current(columns, contents.size());

    std::vector<T> sorted_values = values;
    std::sort(sorted_values.begin(), sorted_values.end(), [](const T& a, const T& b){ return a.date < b.date; });

    for(auto& expense : sorted_values){
        if(expense.date.year() == year && expense.date.month() == month){
            std::size_t index = indexes[get_account(expense.account).name];
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

void month_overview(budget::month month, budget::year year){
    auto accounts = all_accounts(year, month);

    std::cout << "Overview of " << month << " " << year << std::endl << std::endl;

    std::vector<std::string> columns;
    std::unordered_map<std::string, std::size_t> indexes;
    std::vector<std::vector<std::string>> contents;
    std::vector<money> total_expenses(accounts.size(), budget::money());
    std::vector<money> total_earnings(accounts.size(), budget::money());

    for(auto& account : accounts){
        indexes[account.name] = columns.size();
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

void month_overview(budget::month month){
    auto today = budget::local_day();

    month_overview(month, today.year());
}

void month_overview(){
    auto today = budget::local_day();

    month_overview(today.month(), today.year());
}

template<typename Functor>
void aggregate_overview(bool full, bool disable_groups, const std::string& separator, Functor&& func){
    std::unordered_map<std::string, std::unordered_map<std::string, budget::money>> acc_expenses;

    //Accumulate all the expenses
    for(auto& expense : all_expenses()){
        if(func(expense)){
            auto name = expense.name;

            if(name[name.size() - 1] == ' '){
                name.erase(name.size() - 1, name.size());
            }

            if(!disable_groups){
                auto loc = name.find(separator);
                if(loc != std::string::npos){
                    name = name.substr(0, loc);
                }
            }

            if(full){
                acc_expenses["All accounts"][name] += expense.amount;
            } else {
                auto& account = get_account(expense.account);
                acc_expenses[account.name][name] += expense.amount;
            }
        }
    }

    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    std::vector<std::size_t> current(0, columns.size());

    for(auto& entry: acc_expenses){
        auto column = columns.size();
        columns.push_back(entry.first);
        std::size_t row = 0;

        auto& expenses = entry.second;

        typedef std::pair<std::string, budget::money> s_expense;
        std::vector<s_expense> sorted_expenses;

        for(auto& expense : expenses){
            sorted_expenses.push_back(std::make_pair(expense.first, expense.second));
        }

        std::sort(sorted_expenses.begin(), sorted_expenses.end(),
            [](const s_expense& a, const s_expense& b){ return a.second > b.second; });

        for(auto& expense : sorted_expenses){
            if(contents.size() <= row){
                contents.emplace_back(acc_expenses.size() * 2, "");
            }

            contents[row][column * 2] = expense.first;
            contents[row][column * 2 + 1] = to_string(expense.second);

            ++row;
        }
    }

    display_table(columns, contents, 2);

    std::cout << std::endl;
}

void aggregate_year_overview(bool full, bool disable_groups, const std::string& separator, budget::year year){
    if(invalid_accounts(year)){
        throw budget::budget_exception("The accounts of the different months have different names, impossible to generate the year overview. ");
    }

    std::cout << "Aggregate overview of " << year << std::endl << std::endl;

    aggregate_overview(full, disable_groups, separator, [year](const budget::expense& expense){ return expense.date.year() == year; });
}

void aggregate_month_overview(bool full, bool disable_groups, const std::string& separator, budget::month month, budget::year year){
    std::cout << "Aggregate overview of " << month << " " << year << std::endl << std::endl;

    aggregate_overview(full, disable_groups, separator, [month,year](const budget::expense& expense){ return expense.date.month() == month && expense.date.year() == year; });
}

void year_overview(budget::year year){
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
    auto today = budget::local_day();

    year_overview(today.year());
}

void add_month_columns(std::vector<std::string>& columns, budget::month sm){
    for(unsigned short i = sm; i < 13; ++i){
        budget::month m = i;

        columns.emplace_back(m.as_long_string());
    }
}

int get_current_months(budget::year year){
    auto sm = start_month(year);
    auto current_months = 12 - sm + 1;

    auto today = budget::local_day();

    if(today.year() == year){
        current_months = today.month() - sm + 1;
    }

    return current_months;
}

template<bool Mean = false, bool CMean = false>
inline void generate_total_line(std::vector<std::vector<std::string>>& contents, std::vector<budget::money>& totals, budget::year year, budget::month sm){
    std::vector<std::string> last_row;
    last_row.push_back("Total");

    auto current_months = get_current_months(year);

    budget::money total_total;
    budget::money current_total;
    for(unsigned short i = sm; i < 13; ++i){
        auto total = totals[i - 1];

        last_row.push_back(format_money(total));

        total_total += total;

        budget::month m = i;
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

template<typename T>
void display_values(budget::year year, const std::string& title, const std::vector<T>& values, bool current = true, bool relaxed = true){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(year);
    auto months = 12 - sm + 1;
    auto current_months = get_current_months(year);

    columns.push_back(title);
    add_month_columns(columns, sm);
    columns.push_back("Total");
    columns.push_back("Mean");

    if(current){
        columns.push_back("C. Total");
        columns.push_back("C. Mean");
    }

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
        budget::month m = j;

        for(auto& account : all_accounts(year, m)){
            budget::money month_total;

            for(auto& value : values){
                if(relaxed){
                    if(get_account(value.account).name == account.name && value.date.year() == year && value.date.month() == m){
                        month_total += value.amount;
                    }
                } else {
                    if(value.account == account.id && value.date.year() == year && value.date.month() == m){
                        month_total += value.amount;
                    }
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

        if(current){
            contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name]));
            contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name] / current_months));
        }
    }

    //Generate the final total line

    if(current){
        generate_total_line<true, true>(contents, totals, year, sm);
    } else {
        generate_total_line<true, false>(contents, totals, year, sm);
    }

    display_table(columns, contents);
}

} // end of anonymous namespace
    
constexpr const std::array<std::pair<const char*, const char*>, 1> budget::module_traits<budget::overview_module>::aliases;

void budget::overview_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::overview_module::handle(std::vector<std::string>& args){
    if(all_accounts().empty()){
        throw budget_exception("No accounts defined, you should start by defining some of them");
    }

    if(args.empty() || args.size() == 1){
        month_overview();
    } else {
        auto& subcommand = args[1];

        auto today = budget::local_day();

        if(subcommand == "month"){
            if(args.size() == 2){
                month_overview();
            } else if(args.size() == 3){
                month_overview(budget::month(to_number<unsigned short>(args[2])));
            } else if(args.size() == 4){
                month_overview(
                    budget::month(to_number<unsigned short>(args[2])),
                    budget::year(to_number<unsigned short>(args[3])));
            } else {
                throw budget_exception("Too many arguments to overview month");
            }
        } else if(subcommand == "year"){
            if(args.size() == 2){
                year_overview();
            } else if(args.size() == 3){
                year_overview(budget::year(to_number<unsigned short>(args[2])));
            } else {
                throw budget_exception("Too many arguments to overview month");
            }
        } else if(subcommand == "aggregate"){
            //Default values
            bool full = false;
            bool disable_groups = false;
            std::string separator = "/";

            //Get defaults from config

            if(budget::config_contains("aggregate_full")){
                if(budget::config_value("aggregate_full") == "true"){
                    full = true;
                }
            }

            if(budget::config_contains("aggregate_no_group")){
                if(budget::config_value("aggregate_no_group") == "true"){
                    disable_groups = true;
                }
            }

            if(budget::config_contains("aggregate_separator")){
                separator = budget::config_value("aggregate_separator");
            }

            //Command-line  overrides config

            if(budget::option("--full", args)){
                full = true;
            }

            if(budget::option("--no-group", args)){
                disable_groups = true;
            }

            auto sep_value = budget::option_value("--separator", args, "");
            if(!sep_value.empty()){
                separator = sep_value;
            }

            if(args.size() == 2){
                aggregate_year_overview(full, disable_groups, separator, today.year());
            } else if(args.size() == 3 || args.size() == 4 || args.size() == 5){
                if(args[2] == "year"){
                    if(args.size() == 3){
                        aggregate_year_overview(full, disable_groups, separator, today.year());
                    } else if(args.size() == 4){
                        aggregate_year_overview(full, disable_groups, separator, budget::year(to_number<unsigned short>(args[3])));
                    }
                } else if(args[2] == "month"){
                    if(args.size() == 3){
                        aggregate_month_overview(full, disable_groups, separator, today.month(), today.year());
                    } else if(args.size() == 4){
                        aggregate_month_overview(full, disable_groups, separator, 
                            budget::month(to_number<unsigned short>(args[3])), 
                            today.year());
                    } else if(args.size() == 5){
                        aggregate_month_overview(full, disable_groups, separator, 
                            budget::month(to_number<unsigned short>(args[3])), 
                            budget::year(to_number<unsigned short>(args[4]))
                            );
                    }
                } else {
                    throw budget_exception("Invalid subcommand");
                }
            } else {
                throw budget_exception("Too many arguments to overview aggregate");
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::display_expenses(budget::year year, bool current, bool relaxed){
    display_values(year, "Expenses", all_expenses(), current, relaxed);
}

void budget::display_earnings(budget::year year, bool current, bool relaxed){
    display_values(year, "Earnings", all_earnings(), current, relaxed);
}

void budget::display_local_balance(budget::year year, bool current, bool relaxed){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(year);
    auto months = 12 - sm + 1;
    auto current_months = get_current_months(year);

    columns.push_back("Local Balance");
    add_month_columns(columns, sm);
    columns.push_back("Total");
    columns.push_back("Mean");

    if(current){
        columns.push_back("C. Total");
        columns.push_back("C. Mean");
    }

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
        budget::month m = i;

        for(auto& account : all_accounts(year, m)){
            budget::money total_expenses;
            budget::money total_earnings;

            if(relaxed){
                total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](budget::expense& e){return get_account(e.account).name == account.name && e.date.year() == year && e.date.month() == m;});
                total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](budget::earning& e){return get_account(e.account).name == account.name && e.date.year() == year && e.date.month() == m;});
            } else {
                total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](budget::expense& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
                total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](budget::earning& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
            }

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

        if(current){
            contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name]));
            contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name] / current_months));
        }
    }

    //Generate the total final line

    if(current){
        generate_total_line<true, true>(contents, totals, year, sm);
    } else {
        generate_total_line<true, false>(contents, totals, year, sm);
    }

    display_table(columns, contents);
}

void budget::display_balance(budget::year year, bool, bool relaxed){
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

    auto today = budget::local_day();
    if(year > today.year()){
        auto pretotal = compute_total_budget(sm, year);
        std::size_t i = 0;
        for(auto& account : all_accounts(year, sm)){
            account_previous[account.name][sm - 1] += pretotal[i++] - account.amount;
        }
    }

    //Fill the table

    for(unsigned short i = sm; i < 13; ++i){
        budget::month m = i;

        for(auto& account : all_accounts(year, m)){
            budget::money total_expenses;
            budget::money total_earnings;

            if(relaxed){
                total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](budget::expense& e){return get_account(e.account).name == account.name && e.date.year() == year && e.date.month() == m;});
                total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](budget::earning& e){return get_account(e.account).name == account.name && e.date.year() == year && e.date.month() == m;});
            } else {
                total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](budget::expense& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
                total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](budget::earning& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
            }

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

