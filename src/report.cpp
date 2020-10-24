//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "report.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "accounts.hpp"
#include "console.hpp"
#include "writer.hpp"
#include "date.hpp"

using namespace budget;

namespace {
using graph_type = std::vector<std::vector<std::string>>;

void render(budget::writer& w, graph_type& graph) {
    std::reverse(graph.begin(), graph.end());

    for (auto& line : graph) {
        for (auto& col : line) {
            w << col;
        }

        w << end_of_line;
    }
}

void write(graph_type& graph, int row, int col, const std::string& value) {
    for (size_t i = 0; i < value.size(); ++i) {
        graph[row][col + i] = value[i];
    }
}


} //end of anonymous namespace

void budget::report_module::load() {
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::report_module::handle(const std::vector<std::string>& args) {
    auto today = budget::local_day();

    budget::console_writer w(std::cout);

    if (args.size() == 1) {
        report(w, today.year(), false, "");
    } else {
        auto& subcommand = args[1];

        if (subcommand == "monthly") {
            report(w, today.year(), false, "");
        } else if (subcommand == "account") {
            std::string account_name;
            edit_string_complete(account_name, "Account", all_account_names(), not_empty_checker(), account_checker());

            report(w, today.year(), true, account_name);
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::report(budget::writer& w, budget::year year, bool filter, const std::string& filter_account) {
    auto today = budget::local_day();

    auto sm = start_month(year);

    data_cache cache;

    if (w.is_web()) {
        w << title_begin << "Monthly report of " + to_string(year) << title_end;

        std::vector<std::string> categories;
        std::vector<std::string> series_names;
        std::vector<std::vector<float>> series_values;

        series_names.push_back("Expenses");
        series_names.push_back("Earnings");
        series_names.push_back("Balance");

        series_values.emplace_back();
        series_values.emplace_back();
        series_values.emplace_back();

        for (auto i = sm; i <= today.month(); ++i) {
            budget::month month = i;

            //Display month legend
            categories.push_back(month.as_short_string());

            budget::money m_balance;
            budget::money m_expenses;
            budget::money m_earnings;

            for (auto& account : all_accounts(cache, year, month)) {
                if (!filter || account.name == filter_account) {
                    auto expenses = accumulate_amount(all_expenses_month(cache, account.id, year, month));
                    auto earnings = accumulate_amount(all_earnings_month(cache, account.id, year, month));

                    m_expenses += expenses;
                    m_earnings += earnings;
                    m_balance += account.amount - expenses + earnings;
                }
            }

            series_values[0].push_back(static_cast<float>(m_expenses));
            series_values[1].push_back(static_cast<float>(m_earnings));
            series_values[2].push_back(static_cast<float>(m_balance));
        }

        w.display_graph("Monthly report of " + to_string(year), categories, series_names, series_values);

        return;
    }

    budget::money max_expenses;
    budget::money max_earnings;
    budget::money max_balance;
    budget::money min_expenses;
    budget::money min_earnings;
    budget::money min_balance;

    std::vector<int> expenses(12);
    std::vector<int> earnings(12);
    std::vector<int> balances(12);

    for (auto i = sm; i <= today.month(); ++i) {
        budget::month month = i;

        budget::money total_expenses;
        budget::money total_earnings;
        budget::money total_balance;

        for (auto& account : all_accounts(cache, year, month)) {
            if (!filter || account.name == filter_account) {
                auto expenses = accumulate_amount_if(cache.expenses(),
                                                     [year, month, account](const budget::expense& e) { return e.account == account.id && e.date.year() == year && e.date.month() == month; });
                auto earnings = accumulate_amount_if(cache.earnings(),
                                                     [year, month, account](const budget::earning& e) { return e.account == account.id && e.date.year() == year && e.date.month() == month; });

                total_expenses += expenses;
                total_earnings += earnings;

                auto balance = account.amount;
                balance -= expenses;
                balance += earnings;

                total_balance += balance;
            }
        }

        expenses[month - 1] = total_expenses.dollars();
        earnings[month - 1] = total_earnings.dollars();
        balances[month - 1] = total_balance.dollars();

        max_expenses = std::max(max_expenses, total_expenses);
        max_earnings = std::max(max_earnings, total_earnings);
        max_balance  = std::max(max_balance, total_balance);
        min_expenses = std::min(min_expenses, total_expenses);
        min_earnings = std::min(min_earnings, total_earnings);
        min_balance  = std::min(min_balance, total_balance);
    }

    auto max_number = std::max(std::max(
                                   std::max(std::abs(max_expenses.dollars()), std::abs(max_earnings.dollars())),
                                   std::max(std::abs(max_balance.dollars()), std::abs(min_expenses.dollars()))),
                               std::max(std::abs(min_balance.dollars()), std::abs(min_earnings.dollars())));

    size_t height = terminal_height() - 9;
    size_t width  = terminal_width() - 6;

    size_t scale_width = 5;

    // Compute the scale based on the data
    size_t scale = 1;
    if (max_number < 600) {
        scale = 100;
    } else if (max_number < 1500) {
        scale = 200;
    } else if (max_number < 3000) {
        scale = 500;
    } else if (max_number < 6000) {
        scale = 1000;
    } else {
        scale = 2000;
    }

    auto graph_width_func = [sm](size_t col_width) {
        constexpr size_t left_space = 7;
        constexpr size_t legend_width = 20;

        return left_space + (13 - sm) * (3 * col_width + 2) + (13 - sm - 1) * 2 + legend_width;
    };

    // Compute the best column width

    size_t col_width = 2;
    if (width > graph_width_func(4)) {
        col_width = 4;
    } else if (width > graph_width_func(3)) {
        col_width = 3;
    } else if (width > graph_width_func(2)) {
        col_width = 2;
    }

    int min = 0;
    if (min_expenses.negative() || min_earnings.negative() || min_balance.negative()) {
        min = std::min(min_expenses, std::min(min_earnings, min_balance)).dollars();
        min = -1 * ((std::abs(min) / scale) + 1) * scale;
    }

    unsigned int max = std::max(max_earnings, std::max(max_expenses, max_balance)).dollars();
    max              = ((max / scale) + 1) * scale;

    unsigned int levels = max / scale + std::abs(min) / scale;

    unsigned int step_height = height / levels;
    unsigned int precision   = scale / step_height;

    auto graph_height = 9 + step_height * levels;
    auto graph_width  = graph_width_func(col_width);

    graph_type graph(graph_height, std::vector<std::string>(graph_width, " "));

    //Display graph title

    write(graph, graph_height - 2, 8, "Monthly report of " + to_string(year));

    //Display scale

    for (size_t i = 0; i <= levels; ++i) {
        int level = min + i * scale;

        write(graph, 4 + step_height * i, 1, to_string(level));
    }

    //Display bar

    unsigned int min_index  = 3;
    unsigned int zero_index = min_index + 1 + (std::abs(min) / scale) * step_height;

    const auto first_bar = scale_width + 2;

    for (auto i = sm; i <= today.month(); ++i) {
        budget::month month = i;

        auto col_start = first_bar + (3 * col_width + 4) * (i - sm);

        //Display month legend
        auto month_str = month.as_short_string();
        write(graph, 1, col_start + 2, month_str);

        for (size_t j = 0; j < expenses[month - 1] / precision; ++j) {
            for (size_t x = 0; x < col_width; ++x) {
                graph[zero_index + j][col_start + x] = "\033[1;41m \033[0m";
            }
        }

        col_start += col_width + 1;

        for (size_t j = 0; j < earnings[month - 1] / precision; ++j) {
            for (size_t x = 0; x < col_width; ++x) {
                graph[zero_index + j][col_start + x] = "\033[1;42m \033[0m";
            }
        }

        col_start += col_width + 1;

        if (balances[month - 1] >= 0) {
            for (size_t j = 0; j < balances[month - 1] / precision; ++j) {
                for (size_t x = 0; x < col_width; ++x) {
                    graph[zero_index + j][col_start + x] = "\033[1;44m \033[0m";
                }
            }
        } else {
            for (size_t j = 0; j < std::abs(balances[month - 1]) / precision; ++j) {
                for (size_t x = 0; x < col_width; ++x) {
                    graph[zero_index - 1 - j][col_start + x] = "\033[1;44m \033[0m";
                }
            }
        }
    }

    //Display legend

    int start_legend = first_bar + (3 * col_width + 4) * (today.month() - sm + 1) + 4;

    graph[4][start_legend - 2] = "|";
    graph[3][start_legend - 2] = "|";
    graph[2][start_legend - 2] = "|";

    graph[4][start_legend] = "\033[1;41m \033[0m";
    graph[3][start_legend] = "\033[1;42m \033[0m";
    graph[2][start_legend] = "\033[1;44m \033[0m";

    write(graph, 6, start_legend - 2, " ____________ ");
    write(graph, 5, start_legend - 2, "|            |");
    write(graph, 4, start_legend + 2, "Expenses |");
    write(graph, 3, start_legend + 2, "Earnings |");
    write(graph, 2, start_legend + 2, "Balance  |");
    write(graph, 1, start_legend - 2, "|____________|");

    //Render the graph

    render(w, graph);
}
