//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "expenses.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"

using namespace budget;

static data_handler<expense> expenses;

int budget::handle_expenses(const std::vector<std::string>& args){
    load_expenses();

    if(args.size() == 1){
        show_expenses();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "show"){
            show_expenses();
        } else if(subcommand == "add"){
            if(args.size() < 4){
                std::cout << "Not enough args for expense add" << std::endl;

                return 1;
            }

            expense expense;
            expense.guid = generate_guid();
            expense.expense_time = boost::posix_time::second_clock::local_time();

            std::string amount_string = args[2];
            expense.amount = parse_money(amount_string);

            if(expense.amount.dollars < 0 || expense.amount.cents < 0){
                std::cout << "Amount of the expense cannot be negative" << std::endl;

                return 1;
            }

            for(std::size_t i = 3; i < args.size(); ++i){
                expense.name += args[i] + " ";
            }

            add_data(expenses, std::move(expense));
        } else if(subcommand == "delete"){
            std::size_t id = to_number<std::size_t>(args[2]);

            if(exists(expenses, id)){
                remove(expenses, id);

                std::cout << "Expense " << id << " has been deleted" << std::endl;
            } else {
                std::cout << "There are no expense with id " << id << std::endl;

                return 1;
            }
        } else {
            std::cout << "Invalid subcommand \"" << subcommand << "\"" << std::endl;

            return 1;
        }
    }

    save_expenses();

    return 0;
}

void budget::load_expenses(){
    load_data(expenses, "expenses.data");
}

void budget::save_expenses(){
    save_data(expenses, "expenses.data");
}

void budget::show_expenses(){
    std::vector<std::string> columns = {"ID", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    //TODO Filter by the current month

    money total;

    //TODO Display date
    for(auto& expense : expenses.data){
        contents.push_back({to_string(expense.id), expense.name, to_string(expense.amount)});

        total += expense.amount;
    }

    contents.push_back({"", "Total", to_string(total)});

    display_table(columns, contents);
}

std::ostream& budget::operator<<(std::ostream& stream, const expense& expense){
    return stream << expense.id  << ':' << expense.guid << ':' << expense.name << ':' << expense.amount << ':' << boost::posix_time::to_iso_string(expense.expense_time);
}

void budget::operator>>(const std::vector<std::string>& parts, expense& expense){
    expense.id = to_number<std::size_t>(parts[0]);
    expense.guid = parts[1];
    expense.name = parts[2];
    expense.amount = parse_money(parts[3]);
    expense.expense_time = boost::posix_time::from_iso_string(parts[4]);
}
