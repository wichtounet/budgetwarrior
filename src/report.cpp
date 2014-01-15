//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "report.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "accounts.hpp"

using namespace budget;

namespace {

typedef std::vector<std::vector<std::string>> graph_type;

void render(graph_type& graph){
    std::reverse(graph.begin(), graph.end());

    for(auto& line : graph){
        for(auto& col : line){
            std::cout << col;
        }
        std::cout << std::endl;
    }
}

void write(graph_type& graph, int row, int col, const std::string& value){
    for(std::size_t i = 0; i < value.size(); ++i){
        graph[row][col + i] = value[i];
    }
}

void monthly_report(boost::gregorian::greg_year year){
    auto today = boost::gregorian::day_clock::local_day();

    budget::money max_expenses;
    budget::money max_earnings;
    budget::money max_balance;
    budget::money min_expenses;
    budget::money min_earnings;
    budget::money min_balance;

    auto sm = start_month(year);

    std::vector<int> expenses(13);
    std::vector<int> earnings(13);
    std::vector<int> balances(13);

    for(unsigned short i = sm + 1; i < today.month() + 1; ++i){
        boost::gregorian::greg_month month = i;

        budget::money total_expenses;
        budget::money total_earnings;
        budget::money total_balance;

        for(auto& account : all_accounts(year, month)){
            auto expenses = accumulate_amount_if(all_expenses(),
                [year,month,account](budget::expense& e){return e.account == account.id && e.date.year() == year && e.date.month() == month;});
            auto earnings = accumulate_amount_if(all_earnings(),
                [year,month,account](budget::earning& e){return e.account == account.id && e.date.year() == year && e.date.month() == month;});

            total_expenses += expenses;
            total_earnings += earnings;

            auto balance = account.amount;
            balance -= expenses;
            balance += earnings;

            total_balance += balance;
        }

        expenses[month] = total_expenses.dollars();
        earnings[month] = total_earnings.dollars();
        balances[month] = total_balance.dollars();

        max_expenses = std::max(max_expenses, total_expenses);
        max_earnings = std::max(max_earnings, total_earnings);
        max_balance = std::max(max_balance, total_balance);
        min_expenses = std::min(min_expenses, total_expenses);
        min_earnings = std::min(min_earnings, total_earnings);
        min_balance = std::min(min_balance, total_balance);
    }

    auto height = terminal_height() - 9;
    auto width = terminal_width() - 6;

    //TODO compute the scale based on the data
    unsigned int scale = 1000;
    unsigned int scale_width = 5;

    int min = 0;
    if(min_expenses.negative() || min_earnings.negative() || min_balance.negative()){
        min = std::min(min_expenses, std::min(min_earnings, min_balance)).dollars();
        min = -1 * ((std::abs(min) / scale) + 1) * scale;
    }

    unsigned int max = std::max(max_earnings, std::max(max_expenses, max_balance)).dollars();
    max = ((max / scale) + 1) * scale;

    unsigned int levels = max / scale + std::abs(min) / scale;

    unsigned int step_height = height / levels;
    unsigned int precision = scale / step_height;

    auto graph_height = 9 + step_height * levels;
    auto graph_width = 6 + (13 - sm) * 8 + (13 - sm - 1) * 2;

    graph_type graph(graph_height, std::vector<std::string>(graph_width, " "));

    //Display graph title

    write(graph, graph_height - 2, 8, "Monthly report of " + to_string(year));

    //Display scale

    int first_step = min == 0 ? 0 : -1 * ((-min % scale) + 1) * scale;

    for(int i = 0; i <= levels; ++i){
        int level = first_step + i * scale;

        write(graph, 4 + step_height * i, 1, to_string(level));
    }

    //Display bar

    unsigned int min_index = 3;
    unsigned int zero_index = min_index + 1 + (std::abs(min) / scale) * step_height;

    auto first_bar = scale_width + 2;

    //TODO Choose bar width based on the terminal width

    for(unsigned short i = sm + 1; i < today.month() + 1; ++i){
        boost::gregorian::greg_month month = i;

        auto col_start = first_bar + 10 * (i - sm - 1);

        //Display month legend
        auto month_str = month.as_short_string();
        write(graph, 1, col_start + 2, month_str);

        for(std::size_t j = 0; j < expenses[month] / precision; ++j){
           graph[zero_index + j][col_start] = "\033[1;41m \033[0m";
           graph[zero_index + j][col_start + 1] = "\033[1;41m \033[0m";
        }

        col_start += 3;

        for(std::size_t j = 0; j < earnings[month] / precision; ++j){
           graph[zero_index + j][col_start] = "\033[1;42m \033[0m";
           graph[zero_index + j][col_start + 1] = "\033[1;42m \033[0m";
        }

        col_start += 3;

        if(balances[month] >= 0){
            for(std::size_t j = 0; j < balances[month] / precision; ++j){
                graph[zero_index + j][col_start] = "\033[1;44m \033[0m";
                graph[zero_index + j][col_start + 1] = "\033[1;44m \033[0m";
            }
        } else {
            for(std::size_t j = 0; j < std::abs(balances[month]) / precision; ++j){
                graph[zero_index - 1 - j][col_start] = "\033[1;44m \033[0m";
                graph[zero_index - 1 - j][col_start + 1] = "\033[1;44m \033[0m";
            }
        }
    }

    //Display legend

    int start_legend = first_bar + 10 * (today.month() - sm) + 4;

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

    render(graph);
}

} //end of anonymous namespace

void budget::report_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::report_module::handle(const std::vector<std::string>& args){
    auto today = boost::gregorian::day_clock::local_day();

    if(args.size() == 1){
        monthly_report(today.year());
    } else {
        auto& subcommand = args[1];

        if(subcommand == "monthly"){
            monthly_report(today.year());
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}
