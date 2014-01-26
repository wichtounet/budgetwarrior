//=======================================================================
// Copyright (c) 2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "objectives.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "fortune.hpp"
#include "accounts.hpp"
#include "args.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "budget_exception.hpp"

using namespace budget;

namespace {

static data_handler<objective> objectives;

void list_objectives(){
    if(objectives.data.size() == 0){
        std::cout << "No objectives" << std::endl;
    } else {
        std::vector<std::string> columns = {"ID", "Name", "Type", "Source", "Operator", "Amount"};
        std::vector<std::vector<std::string>> contents;

        for(auto& objective : objectives.data){
            contents.push_back({to_string(objective.id), objective.name, objective.type, objective.source, objective.op, to_string(objective.amount)});
        }

        display_table(columns, contents);
    }
}

void print_success(budget::money balance, budget::money earnings, budget::money expenses, const budget::objective& objective){
    auto success = compute_success(balance, earnings, expenses, objective);

    if(success < 25){
        std::cout << "\033[0;31m";
    } else if(success < 75){
        std::cout << "\033[0;33m";
    } else if(success < 100){
        std::cout << "\033[0;32m";
    } else if(success >= 100){
        std::cout << "\033[1;32m";
    }

    print_minimum(success, 5);
    std::cout << "%\033[0m  ";

    success = std::min(success, 109);
    auto good = success == 0 ? 0 : (success / 10) + 1;

    for(std::size_t i = 0; i < good; ++i){
        std::cout << "\033[1;42m   \033[0m";
    }

    for(std::size_t i = good; i < 11; ++i){
        std::cout << "\033[1;41m   \033[0m";
    }
}

void status_objectives(){
    if(objectives.data.size() == 0){
        std::cout << "No objectives" << std::endl;
    } else {
        auto today = boost::gregorian::day_clock::local_day();
        auto current_month = today.month();
        auto current_year = today.year();
        auto sm = start_month(current_year);

        size_t monthly = 0;
        size_t yearly = 0;

        for(auto& objective : objectives.data){
            if(objective.type == "yearly"){
                ++yearly;
            } else if(objective.type == "monthly"){
                ++monthly;
            }
        }

        if(yearly){
            std::cout << "Year objectives" << std::endl << std::endl;

            size_t width = 0;
            for(auto& objective : objectives.data){
                if(objective.type == "yearly"){
                    width = std::max(rsize(objective.name), width);
                }
            }

            budget::money expenses;
            for(auto& expense : all_expenses()){
                if(expense.date.year() == current_year && expense.date.month() >= sm && expense.date.month() <= current_month){
                    expenses += expense.amount;
                }
            }

            budget::money earnings;
            for(auto& earning : all_earnings()){
                if(earning.date.year() == current_year && earning.date.month() >= sm && earning.date.month() <= current_month){
                    earnings += earning.amount;
                }
            }

            auto balance = earnings - expenses;
            for(unsigned short i = sm; i <= current_month; ++i){
                boost::gregorian::greg_month month = i;

                auto current_accounts = all_accounts(current_year, month);

                for(auto& c : current_accounts){
                    balance += c.amount;
                }
            }

            for(auto& objective : objectives.data){
                if(objective.type == "yearly"){
                    std::cout << "  ";
                    print_minimum(objective.name, width);
                    std::cout << "  ";

                    print_success(balance, earnings, expenses, objective);

                    std::cout << std::endl;
                }
            }

            if(monthly){
                std::cout << std::endl;
            }
        }

        if(monthly){
            std::cout << "Month objectives" << std::endl;

            size_t width = 0;
            for(unsigned short i = sm; i <= current_month; ++i){
                boost::gregorian::greg_month month = i;

                std::stringstream stream;
                stream << month;

                width = std::max(width, stream.str().size());
            }

            for(auto& objective : objectives.data){
                if(objective.type == "monthly"){
                    std::cout << std::endl << objective.name << std::endl;

                    size_t width = 0;
                    for(unsigned short i = sm; i <= current_month; ++i){
                        boost::gregorian::greg_month month = i;

                        std::cout << "  ";
                        print_minimum(month, width);
                        std::cout << "  ";

                        budget::money expenses;
                        for(auto& expense : all_expenses()){
                            if(expense.date.year() == current_year && expense.date.month() == month){
                                expenses += expense.amount;
                            }
                        }

                        budget::money earnings;
                        for(auto& earning : all_earnings()){
                            if(earning.date.year() == current_year && earning.date.month() == month){
                                earnings += earning.amount;
                            }
                        }

                        auto balance = earnings - expenses;
                        for(auto& c : all_accounts(current_year, month)){
                            balance += c.amount;
                        }

                        print_success(balance, earnings, expenses, objective);

                        std::cout << std::endl;
                    }
                }
            }
        }
    }
}

void edit(budget::objective& objective){
    edit_string(objective.name, "Name");
    not_empty(objective.name, "The name of the objective cannot be empty");

    edit_string(objective.type, "Type");
    not_empty(objective.type, "The type of the objective cannot be empty");
    one_of(objective.type, "Possible values for type are [monthly,yearly]", {"monthly","yearly"});

    edit_string(objective.source, "Source");
    not_empty(objective.source, "The source of the objective cannot be empty");
    one_of(objective.type, "Possible values for source are [balance,earnings,expenses]", {"balance", "earnings", "expenses"});

    edit_string(objective.op, "Operator");
    not_empty(objective.op, "The operator of the objective cannot be empty");
    one_of(objective.type, "Possible values for operator are [min,max]", {"min", "max"});

    edit_money(objective.amount, "Amount");
}

} //end of anonymous namespace

int budget::compute_success(budget::money balance, budget::money earnings, budget::money expenses, const budget::objective& objective){
    auto amount = objective.amount;

    budget::money basis;
    if(objective.source == "expenses"){
        basis = expenses;
    } else if (objective.source == "earnings") {
        basis = earnings;
    } else {
        basis = balance;
    }

    int success = 0;
    if(objective.op == "min"){
        auto percent = basis.dollars() / static_cast<double>(amount.dollars());
        success = percent * 100;
    } else if(objective.op == "max"){
        auto percent = amount.dollars() / static_cast<double>(basis.dollars());
        success = percent * 100;
    }

    success = std::max(0, success);

    return success;
}

void budget::objectives_module::load(){
    load_expenses();
    load_earnings();
    load_accounts();
    load_fortunes();
    load_objectives();
}

void budget::objectives_module::unload(){
    save_objectives();
}

void budget::objectives_module::handle(const std::vector<std::string>& args){
    if(args.size() == 1){
        status_objectives();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "list"){
            list_objectives();
        } else if(subcommand == "status"){
            status_objectives();
        } else if(subcommand == "add"){
            objective objective;
            objective.guid = generate_guid();
            objective.date = boost::gregorian::day_clock::local_day();

            edit(objective);

            add_data(objectives, std::move(objective));
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(objectives, id)){
                throw budget_exception("There are no objective with id ");
            }

            remove(objectives, id);

            std::cout << "Objective " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(objectives, id)){
                throw budget_exception("There are no objective with id " + args[2]);
            }

            auto& objective = get(objectives, id);

            edit(objective);

            set_objectives_changed();
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_objectives(){
    load_data(objectives, "objectives.data");
}

void budget::save_objectives(){
    save_data(objectives, "objectives.data");
}

void budget::add_objective(budget::objective&& objective){
    add_data(objectives, std::forward<budget::objective>(objective));
}

std::ostream& budget::operator<<(std::ostream& stream, const objective& objective){
    return stream
        << objective.id  << ':'
        << objective.guid << ':'
        << objective.name << ':'
        << objective.type << ':'
        << objective.source << ':'
        << objective.op << ':'
        << objective.amount << ':'
        << to_string(objective.date);
}

void budget::operator>>(const std::vector<std::string>& parts, objective& objective){
    objective.id = to_number<std::size_t>(parts[0]);
    objective.guid = parts[1];
    objective.name = parts[2];
    objective.type = parts[3];
    objective.source = parts[4];
    objective.op = parts[5];
    objective.amount = parse_money(parts[6]);
    objective.date = boost::gregorian::from_string(parts[7]);
}

std::vector<objective>& budget::all_objectives(){
    return objectives.data;
}

void budget::set_objectives_changed(){
    objectives.changed = true;
}
