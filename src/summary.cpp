//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
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
#include "fortune.hpp"
#include "objectives.hpp"
#include "budget_exception.hpp"
#include "config.hpp"
#include "utils.hpp"

using namespace budget;

namespace {

std::string account_summary(budget::month month, budget::year year){
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

    budget::money prev_expenses;
    budget::money prev_earnings;
    budget::money prev_balance;
    budget::money prev_local;

    for (unsigned short i = sm; i <= month; ++i) {
        budget::month m = i;

        for (auto& account : all_accounts(year, m)) {
            auto total_expenses = accumulate_amount_if(all_expenses(), [account, year, m](budget::expense& e) { return e.account == account.id && e.date.year() == year && e.date.month() == m; });
            auto total_earnings = accumulate_amount_if(all_earnings(), [account, year, m](budget::earning& e) { return e.account == account.id && e.date.year() == year && e.date.month() == m; });

            auto balance       = account_previous[account.name] + account.amount - total_expenses + total_earnings;
            auto local_balance = account.amount - total_expenses + total_earnings;

            if (i == month) {
                contents.push_back({account.name});

                contents.back().push_back(format_money(total_expenses));
                contents.back().push_back(format_money(total_earnings));
                contents.back().push_back(format_money(balance));
                contents.back().push_back(format_money(local_balance));

                tot_expenses += total_expenses;
                tot_earnings += total_earnings;
                tot_balance += balance;
                tot_local += local_balance;
            } else if(month > 1 && m == month - 1){
                prev_expenses = total_expenses;
                prev_earnings = total_earnings;
                prev_balance  = balance;
                prev_local    = local_balance;
            }

            account_previous[account.name] = balance;
        }
    }

    contents.push_back({"Total"});

    contents.back().push_back(format_money(tot_expenses));
    contents.back().push_back(format_money(tot_earnings));
    contents.back().push_back(format_money(tot_balance));
    contents.back().push_back(format_money(tot_local));

    if(month > 1){
        contents.push_back({"Previous"});

        contents.back().push_back(format_money(prev_expenses));
        contents.back().push_back(format_money(prev_earnings));
        contents.back().push_back(format_money(prev_balance));
        contents.back().push_back(format_money(prev_local));

        contents.push_back({"Average"});

        contents.back().push_back(format_money(tot_expenses / month.value));
        contents.back().push_back(format_money(tot_earnings / month.value));
        contents.back().push_back(format_money(tot_balance / month.value));
        contents.back().push_back(format_money(tot_local / month.value));
    }

    std::stringstream ss;
    display_table(ss, columns, contents);
    return ss.str();
}

std::string objectives_summary(){
    std::stringstream ss;
    budget::yearly_objective_status(ss, false);
    ss << std::endl;
    budget::current_monthly_objective_status(ss);
    return ss.str();
}

std::string fortune_summary(){
    std::stringstream ss;
    budget::status_fortunes(ss, true);
    return ss.str();
}

void print_columns(const std::vector<std::string>& columns){
    std::vector<std::vector<std::string>> split_columns;

    for(auto& column : columns){
        split_columns.push_back(budget::split(column, '\n'));
    }

    std::vector<std::string> composed_columns;

    for(auto& column : split_columns){
        size_t max_width = 0;

        for(auto& row : column){
            max_width = std::max(max_width, rsize_after(row));
        }

        for(size_t i = 0; i < column.size(); ++i){
            auto& row = column[i];

            if(i >= composed_columns.size()){
                composed_columns.emplace_back();
            }

            composed_columns[i] += row;

            auto length = rsize_after(row);

            if(length < max_width){
                composed_columns[i] += std::string(max_width - length, ' ');
            }
        }

        for(size_t i = column.size(); i < composed_columns.size(); ++i){
            composed_columns[i] += std::string(max_width, ' ');
        }

        // Add some spacing from previous column
        for(auto& row : composed_columns){
            row += " ";
        }
    }

    for(auto& row : composed_columns){
        std::cout << row << std::endl;
    }
}

void month_overview(budget::month month, budget::year year) {
    // First display overview of the accounts
    auto m_summary = account_summary(month, year);

    // Second display a summary of the objectives
    auto m_objectives_summary = objectives_summary();

    // Third display a summary of the fortune
    auto m_fortune_summary = fortune_summary();

    print_columns({m_summary, m_objectives_summary, m_fortune_summary});
}

void month_overview(budget::month m) {
    month_overview(m, local_day().year());
}

void month_overview() {
    month_overview(local_day().month(), local_day().year());
}

} // end of anonymous namespace

constexpr const std::array<std::pair<const char*, const char*>, 1> budget::module_traits<budget::summary_module>::aliases;

void budget::summary_module::load() {
    load_accounts();
    load_expenses();
    load_earnings();
    load_objectives();
    load_fortunes();
}

void budget::summary_module::handle(std::vector<std::string>& args) {
    if (all_accounts().empty()) {
        throw budget_exception("No accounts defined, you should start by defining some of them");
    }

    if (args.empty() || args.size() == 1) {
        month_overview();
    } else {
        auto& subcommand = args[1];

        if (subcommand == "month") {
            auto today = local_day();

            if (args.size() == 2) {
                month_overview();
            } else if (args.size() == 3) {
                auto m = budget::month(to_number<unsigned short>(args[2]));

                if (m > today.month()) {
                    throw budget_exception("Cannot compute the summary of the future");
                }

                month_overview(m);
            } else if (args.size() == 4) {
                auto m = budget::month(to_number<unsigned short>(args[2]));
                auto y = budget::year(to_number<unsigned short>(args[3]));

                if (y > today.year()) {
                    throw budget_exception("Cannot compute the summary of the future");
                }

                if (y == today.year() && m > today.month()) {
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
