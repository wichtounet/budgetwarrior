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
//#include <boost/date_time/date_parsing.hpp>

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
            if(args.size() == 2){
                show_expenses();
            } else if(args.size() == 3){
                show_expenses(boost::gregorian::greg_month(to_number<unsigned short>(args[2])));
            } else if(args.size() == 4){
                show_expenses(
                    boost::gregorian::greg_month(to_number<unsigned short>(args[2])),
                    boost::gregorian::greg_year(to_number<unsigned short>(args[3])));
            } else {
                std::cout << "Too many arguments to expense show" << std::endl;

                return 1;
            }
        } else if(subcommand == "all"){
            all_expenses();
        } else if(subcommand == "add"){
            if(args.size() < 4){
                std::cout << "Not enough args for expense add" << std::endl;

                return 1;
            }

            expense expense;
            expense.guid = generate_guid();
            expense.expense_date = boost::gregorian::day_clock::local_day();

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
        } else if(subcommand == "addd"){
            if(args.size() < 5){
                std::cout << "Not enough args for expense add date" << std::endl;

                return 1;
            }

            expense expense;
            expense.guid = generate_guid();
            expense.expense_date = boost::gregorian::from_string(args[2]);

            std::string amount_string = args[3];
            expense.amount = parse_money(amount_string);

            if(expense.amount.dollars < 0 || expense.amount.cents < 0){
                std::cout << "Amount of the expense cannot be negative" << std::endl;

                return 1;
            }

            for(std::size_t i = 4; i < args.size(); ++i){
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
    date today = boost::gregorian::day_clock::local_day();

    show_expenses(today.month(), today.year());
}

void budget::show_expenses(boost::gregorian::greg_month month){
    date today = boost::gregorian::day_clock::local_day();

    show_expenses(month, today.year());
}

void budget::show_expenses(boost::gregorian::greg_month month, boost::gregorian::greg_year year){
    std::vector<std::string> columns = {"ID", "Date", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    money total;
    std::size_t count = 0;

    for(auto& expense : expenses.data){
        if(expense.expense_date.year() == year && expense.expense_date.month() == month){
            contents.push_back({to_string(expense.id), to_string(expense.expense_date), expense.name, to_string(expense.amount)});

            total += expense.amount;
            ++count;
        }
    }

    if(count == 0){
        std::cout << "No expenses for " << month << "-" << year << std::endl;
    } else {
        contents.push_back({"", "", "Total", to_string(total)});

        display_table(columns, contents);
    }
}

void budget::all_expenses(){
    std::vector<std::string> columns = {"ID", "Date", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    for(auto& expense : expenses.data){
        contents.push_back({to_string(expense.id), to_string(expense.expense_date), expense.name, to_string(expense.amount)});
    }

    display_table(columns, contents);
}

std::ostream& budget::operator<<(std::ostream& stream, const expense& expense){
    return stream << expense.id  << ':' << expense.guid << ':' << expense.name << ':' << expense.amount << ':' << to_string(expense.expense_date);
}

void budget::operator>>(const std::vector<std::string>& parts, expense& expense){
    expense.id = to_number<std::size_t>(parts[0]);
    expense.guid = parts[1];
    expense.name = parts[2];
    expense.amount = parse_money(parts[3]);
    expense.expense_date = boost::gregorian::from_string(parts[4]);
}
