//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "recurring.hpp"
#include "args.hpp"
#include "accounts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "budget_exception.hpp"
#include "expenses.hpp"

using namespace budget;

namespace {

static data_handler<recurring> recurrings;

void show_recurrings(){
    std::vector<std::string> columns = {"ID", "Account", "Name", "Amount", "Recurs"};
    std::vector<std::vector<std::string>> contents;

    money total;

    for(auto& recurring : recurrings.data){
        contents.push_back({to_string(recurring.id), recurring.account, recurring.name, to_string(recurring.amount), recurring.recurs});

        total += recurring.amount;
    }

    if(recurrings.data.empty()){
        std::cout << "No recurring expenses" << std::endl;
    } else {
        contents.push_back({"", "", "", "", ""});
        contents.push_back({"", "", "", "Total", to_string(total)});

        display_table(columns, contents);
    }
}

} //end of anonymous namespace

void budget::recurring_module::preload(){
    load_recurrings();
    load_accounts();

    auto now = budget::local_day();

    //If it does not contains this value, it is the first start, so there is no
    //need to check anything
    if(internal_config_contains("recurring:last_checked")){
        auto last_checked_str = internal_config_value("recurring:last_checked");
        auto last_checked = from_string(last_checked_str);

        if(last_checked.month() < now.month() || last_checked.year() < now.year()){
            load_expenses();

            while(last_checked.year() < now.year() || last_checked.month() < now.month()){
                last_checked += months(1);

                date recurring_date(last_checked.year(), last_checked.month(), 1);

                for(auto& recurring : recurrings.data){
                    budget::expense recurring_expense;
                    recurring_expense.guid = generate_guid();
                    recurring_expense.date = recurring_date;
                    recurring_expense.account = get_account(recurring.account, recurring_date.year(), recurring_date.month()).id;
                    recurring_expense.amount = recurring.amount;
                    recurring_expense.name = recurring.name;

                    add_expense(std::move(recurring_expense));
                }
            }

            save_expenses();
        }
    }

    internal_config_value("recurring:last_checked") = budget::to_string(now);
}

void budget::recurring_module::load(){
    //No need to load anything, that have been done in the preload phase
}

void budget::recurring_module::unload(){
    save_recurrings();
}

void budget::recurring_module::handle(const std::vector<std::string>& args){
    if(args.size() == 1){
        show_recurrings();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "show"){
            show_recurrings();
        } else if(subcommand == "add"){
            recurring recurring;
            recurring.guid = generate_guid();
            recurring.recurs = "monthly";

            //TODO handling of archived accounts is only temporary and not
            //workign properly

            auto date = budget::local_day();

            std::string account_name;
            edit_string(account_name, "Account", not_empty_checker(), account_checker());
            recurring.account = get_account(account_name, date.year(), date.month()).id;

            edit_string(recurring.name, "Name", not_empty_checker());
            edit_money(recurring.amount, "Amount", not_negative_checker());

            auto id = add_data(recurrings, std::move(recurring));
            std::cout << "Recurring expense " << id << " has been created" << std::endl;
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(recurrings, id)){
                throw budget_exception("There are no recurring expense with id " + args[2]);
            }

            remove(recurrings, id);

            std::cout << "Recurring expense " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(recurrings, id)){
                throw budget_exception("There are no recurring expense with id " + args[2]);
            }

            auto& recurring = get(recurrings, id);

            edit_string(recurring.account, "Account", not_empty_checker(), account_checker());
            edit_string(recurring.name, "Name", not_empty_checker());
            edit_money(recurring.amount, "Amount", not_negative_checker());

            recurrings.changed = true;

            std::cout << "Recurring expense " << id << " has been modified" << std::endl;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_recurrings(){
    load_data(recurrings, "recurrings.data");
}

void budget::save_recurrings(){
    save_data(recurrings, "recurrings.data");
}

std::ostream& budget::operator<<(std::ostream& stream, const recurring& recurring){
    return stream << recurring.id  << ':' << recurring.guid << ':' << recurring.account << ':' << recurring.name << ':' << recurring.amount << ":" << recurring.recurs;
}

void budget::migrate_recurring_1_to_2(){
    load_accounts();

    load_data(recurrings, "recurrings.data", [](const std::vector<std::string>& parts, recurring& recurring){
        recurring.id = to_number<std::size_t>(parts[0]);
        recurring.guid = parts[1];
        recurring.old_account = to_number<std::size_t>(parts[2]);
        recurring.name = parts[3];
        recurring.amount = parse_money(parts[4]);
        recurring.recurs = parts[5];
        });

    for(auto& recurring : all_recurrings()){
        recurring.account = get_account(recurring.old_account).name;
    }

    recurrings.changed = true;

    save_data(recurrings, "recurrings.data");
}

void budget::operator>>(const std::vector<std::string>& parts, recurring& recurring){
    recurring.id = to_number<std::size_t>(parts[0]);
    recurring.guid = parts[1];
    recurring.account = parts[2];
    recurring.name = parts[3];
    recurring.amount = parse_money(parts[4]);
    recurring.recurs = parts[5];
}

budget::year budget::first_year(const budget::recurring& recurring){
    budget::year year(1400);

    for(auto& expense : all_expenses()){
        if(expense.name == recurring.name && expense.amount == recurring.amount && recurring.amount == expense.amount){
            if(year == 1400 || expense.date.year() < year){
                year = expense.date.year();
            }
        }
    }

    return year;
}

budget::month budget::first_month(const budget::recurring& recurring, budget::year year){
    budget::month month(13);

    for(auto& expense : all_expenses()){
        if(expense.date.year() == year && expense.name == recurring.name && expense.amount == recurring.amount && recurring.amount == expense.amount){
            if(month == 13 || expense.date.month() < month){
                month = expense.date.month();
            }
        }
    }

    return month;
}

budget::year budget::last_year(const budget::recurring& recurring){
    budget::year year(1400);

    for(auto& expense : all_expenses()){
        if(expense.name == recurring.name && expense.amount == recurring.amount && recurring.amount == expense.amount){
            if(year == 1400 || expense.date.year() > year){
                year = expense.date.year();
            }
        }
    }

    return year;
}

budget::month budget::last_month(const budget::recurring& recurring, budget::year year){
    budget::month month(13);

    for(auto& expense : all_expenses()){
        if(expense.date.year() == year && expense.name == recurring.name && expense.amount == recurring.amount && recurring.amount == expense.amount){
            if(month == 13 || expense.date.month() > month){
                month = expense.date.month();
            }
        }
    }

    return month;
}

std::vector<recurring>& budget::all_recurrings(){
    return recurrings.data;
}

void budget::set_recurrings_changed(){
    recurrings.changed = true;
}

void budget::set_recurrings_next_id(std::size_t next_id){
    recurrings.next_id = next_id;
}
