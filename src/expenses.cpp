//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "expenses.hpp"
#include "args.hpp"
#include "accounts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "budget_exception.hpp"

using namespace budget;

namespace {

static data_handler<expense> expenses;

void show_expenses(budget::month month, budget::year year){
    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    money total;
    std::size_t count = 0;

    for(auto& expense : expenses.data){
        if(expense.date.year() == year && expense.date.month() == month){
            contents.push_back({to_string(expense.id), to_string(expense.date), get_account(expense.account).name, expense.name, to_string(expense.amount)});

            total += expense.amount;
            ++count;
        }
    }

    if(count == 0){
        std::cout << "No expenses for " << month << "-" << year << std::endl;
    } else {
        contents.push_back({"", "", "", "Total", to_string(total)});

        display_table(columns, contents);
    }
}

void show_expenses(budget::month month){
    auto today = budget::local_day();

    show_expenses(month, today.year());
}

void show_expenses(){
    auto today = budget::local_day();

    show_expenses(today.month(), today.year());
}

void show_all_expenses(){
    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    for(auto& expense : expenses.data){
        contents.push_back({to_string(expense.id), to_string(expense.date), get_account(expense.account).name, expense.name, to_string(expense.amount)});
    }

    display_table(columns, contents);
}

} //end of anonymous namespace

void budget::expenses_module::load(){
    load_expenses();
    load_accounts();
}

void budget::expenses_module::unload(){
    save_expenses();
}

void budget::expenses_module::handle(const std::vector<std::string>& args){
    if(args.size() == 1){
        show_expenses();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "show"){
            if(args.size() == 2){
                show_expenses();
            } else if(args.size() == 3){
                show_expenses(budget::month(to_number<unsigned short>(args[2])));
            } else if(args.size() == 4){
                show_expenses(
                    budget::month(to_number<unsigned short>(args[2])),
                    budget::year(to_number<unsigned short>(args[3])));
            } else {
                throw budget_exception("Too many arguments to expense show");
            }
        } else if(subcommand == "all"){
            show_all_expenses();
        } else if(subcommand == "add"){
            expense expense;
            expense.guid = generate_guid();
            expense.date = budget::local_day();

            edit_date(expense.date, "Date");

            std::string account_name;
            edit_string(account_name, "Account", not_empty_checker(), account_checker());
            expense.account = get_account(account_name, expense.date.year(), expense.date.month()).id;

            edit_string(expense.name, "Name", not_empty_checker());
            edit_money(expense.amount, "Amount", not_negative_checker(), not_zero_checker());

            auto id = add_data(expenses, std::move(expense));
            std::cout << "Expense " << id << " has been created" << std::endl;
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(expenses, id)){
                throw budget_exception("There are no expense with id ");
            }

            remove(expenses, id);

            std::cout << "Expense " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(expenses, id)){
                throw budget_exception("There are no expense with id " + args[2]);
            }

            auto& expense = get(expenses, id);

            edit_date(expense.date, "Date");

            auto account_name = get_account(expense.account).name;
            edit_string(account_name, "Account", not_empty_checker(), account_checker());
            expense.account = get_account(account_name, expense.date.year(), expense.date.month()).id;

            edit_string(expense.name, "Name", not_empty_checker());
            edit_money(expense.amount, "Amount", not_negative_checker(), not_zero_checker());

            std::cout << "Expense " << id << " has been modified" << std::endl;

            expenses.changed = true;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_expenses(){
    load_data(expenses, "expenses.data");
}

void budget::save_expenses(){
    save_data(expenses, "expenses.data");
}

void budget::add_expense(budget::expense&& expense){
    add_data(expenses, std::forward<budget::expense>(expense));
}

std::ostream& budget::operator<<(std::ostream& stream, const expense& expense){
    return stream << expense.id  << ':' << expense.guid << ':' << expense.account << ':' << expense.name << ':' << expense.amount << ':' << to_string(expense.date);
}

void budget::operator>>(const std::vector<std::string>& parts, expense& expense){
    expense.id = to_number<std::size_t>(parts[0]);
    expense.guid = parts[1];
    expense.account = to_number<std::size_t>(parts[2]);
    expense.name = parts[3];
    expense.amount = parse_money(parts[4]);
    expense.date = from_string(parts[5]);
}

std::vector<expense>& budget::all_expenses(){
    return expenses.data;
}

void budget::set_expenses_changed(){
    expenses.changed = true;
}
