//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>
#include <unordered_map>
#include <unordered_set>

#include "cpp_utils/assert.hpp"

#include "data_cache.hpp"
#include "summary.hpp"
#include "console.hpp"
#include "compute.hpp"
#include "budget_exception.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

void print_columns(const std::vector<std::string>& columns){
    std::vector<std::vector<std::string>> split_columns;

    for(auto& column : columns){
        split_columns.push_back(budget::split(column, '\n'));
    }

    std::vector<std::string> composed_columns;

    // Compute the total width

    size_t total_max_width = composed_columns.size() * 2;
    size_t max_length = 0;

    for(auto& column : split_columns){
        size_t max_width = 0;

        for(auto& row : column){
            max_width = std::max(max_width, rsize_after(row));
        }

        total_max_width += max_width;

        max_length = std::max(max_length, column.size());
    }

    if (terminal_width() > total_max_width + 3) {
        // By default, print the columns side by side

        for (auto& column : split_columns) {
            size_t max_width = 0;

            for (auto& row : column) {
                max_width = std::max(max_width, rsize_after(row));
            }

            for (size_t i = 0; i < column.size(); ++i) {
                auto& row = column[i];

                if (i >= composed_columns.size()) {
                    composed_columns.emplace_back();
                }

                composed_columns[i] += row;

                auto length = rsize_after(row);

                if (length < max_width) {
                    composed_columns[i] += std::string(max_width - length, ' ');
                }
            }

            for (size_t i = column.size(); i < max_length; ++i) {
                if (i >= composed_columns.size()) {
                    composed_columns.emplace_back();
                }

                composed_columns[i] += std::string(max_width, ' ');
            }

            // Add some spacing from previous column
            for (auto& row : composed_columns) {
                row += " ";
            }
        }

        for (auto& row : composed_columns) {
            std::cout << row << std::endl;
        }
    } else {
        // If the columns cannot be printed side by side
        // print them one after another

        for (auto& column : split_columns) {
            for(auto& row : column){
                std::cout << row << std::endl;
            }

            std::cout << std::endl;
        }
    }
}

void month_overview(budget::month month, budget::year year) {
    // First display overview of the accounts
    std::stringstream summary_ss;
    console_writer summary_w(summary_ss);
    budget::account_summary(summary_w, month, year);
    std::string m_summary = summary_ss.str();

    // Second display a summary of the objectives
    std::stringstream objectives_ss;
    console_writer objectives_w(objectives_ss);
    budget::objectives_summary(objectives_w);
    std::string m_objectives_summary = objectives_ss.str();

    // Third display a summary of the fortune

    std::string m_fortune_summary;

    if(!is_fortune_disabled()){
        std::stringstream fortune_ss;
        console_writer fortune_w(fortune_ss);
        budget::fortune_summary(fortune_w);
        m_fortune_summary = fortune_ss.str();
    }

    // Print each column
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

void budget::account_summary(budget::writer& w, budget::month month, budget::year year){
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

    budget::money month_tot_expenses;
    budget::money month_tot_earnings;
    budget::money month_tot_balance;
    budget::money month_tot_local;

    budget::money tot_expenses;
    budget::money tot_earnings;
    budget::money tot_balance;
    budget::money tot_local;

    budget::money prev_expenses;
    budget::money prev_earnings;
    budget::money prev_balance;
    budget::money prev_local;

    data_cache cache;

    for (unsigned short i = sm; i <= month; ++i) {
        budget::month m = i;

        for (auto& account : all_accounts(cache, year, m)) {
            auto total_expenses = accumulate_amount_if(cache.expenses(), [account, year, m](const budget::expense& e) { return e.account == account.id && e.date.year() == year && e.date.month() == m; });
            auto total_earnings = accumulate_amount_if(cache.earnings(), [account, year, m](const budget::earning& e) { return e.account == account.id && e.date.year() == year && e.date.month() == m; });

            auto balance       = account_previous[account.name] + account.amount - total_expenses + total_earnings;
            auto local_balance = account.amount - total_expenses + total_earnings;

            if (i == month) {
                contents.push_back({account.name});

                contents.back().push_back(format_money(total_expenses));
                contents.back().push_back(format_money(total_earnings));
                contents.back().push_back(format_money(balance));
                contents.back().push_back(format_money(local_balance));

                month_tot_expenses += total_expenses;
                month_tot_earnings += total_earnings;
                month_tot_balance += balance;
                month_tot_local += local_balance;
            } else if (month > 1 && m == month - 1) {
                prev_expenses += total_expenses;
                prev_earnings += total_earnings;
                prev_balance  += balance;
                prev_local    += local_balance;
            }

            tot_expenses += total_expenses;
            tot_earnings += total_earnings;
            tot_balance += balance;
            tot_local += local_balance;

            account_previous[account.name] = balance;
        }
    }

    contents.push_back({"Total"});

    contents.back().push_back(format_money(month_tot_expenses));
    contents.back().push_back(format_money(month_tot_earnings));
    contents.back().push_back(format_money(month_tot_balance));
    contents.back().push_back(format_money(month_tot_local));

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

    w.display_table(columns, contents, 1, {}, 1);
}

void budget::objectives_summary(budget::writer& w){
    budget::yearly_objective_status(w, false, true);
    budget::current_monthly_objective_status(w, true);
}

void budget::fortune_summary(budget::writer& w){
    budget::status_fortunes(w, true);
}
