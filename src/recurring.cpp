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

#include "recurring.hpp"
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

static data_handler<recurring> recurrings;

void show_recurrings(){
    std::vector<std::string> columns = {"ID", "Account", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    money total;

    for(auto& recurring : recurrings.data){
        contents.push_back({to_string(recurring.id), get_account(recurring.account).name, recurring.name, to_string(recurring.amount)});

        total += recurring.amount;
    }

    if(recurrings.data.empty()){
        std::cout << "No recurring expenses" << std::endl;
    } else {
        contents.push_back({"", "", "", ""});
        contents.push_back({"", "", "Total", to_string(total)});

        display_table(columns, contents);
    }
}

} //end of anonymous namespace

void budget::recurring_module::preload(){
    load_recurrings();

}

void budget::recurring_module::load(){
    load_accounts();
    //No need to load recurrings, that have been done in the preload phase
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
            enough_args(args, 5);

            recurring recurring;
            recurring.guid = generate_guid();

            auto account_name = args[2];
            validate_account(account_name);
            recurring.account = get_account(account_name).id;

            recurring.amount = parse_money(args[3]);
            not_negative(recurring.amount);

            for(std::size_t i = 4; i < args.size(); ++i){
                recurring.name += args[i] + " ";
            }

            add_data(recurrings, std::move(recurring));
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(recurrings, id)){
                throw budget_exception("There are no recurring expense with id ");
            }

            remove(recurrings, id);

            std::cout << "Recurring expense " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(recurrings, id)){
                throw budget_exception("There are no recurring expense with id ");
            }

            auto& recurring = get(recurrings, id);

            auto account_name = get_account(recurring .account).name;
            edit_string(account_name, "Account");
            validate_account(account_name);
            recurring.account = get_account(account_name).id;

            edit_string(recurring.name, "Name");
            edit_money(recurring.amount, "Amount");

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
    return stream << recurring.id  << ':' << recurring.guid << ':' << recurring.account << ':' << recurring.name << ':' << recurring.amount;
}

void budget::operator>>(const std::vector<std::string>& parts, recurring& recurring){
    recurring.id = to_number<std::size_t>(parts[0]);
    recurring.guid = parts[1];
    recurring.account = to_number<std::size_t>(parts[2]);
    recurring.name = parts[3];
    recurring.amount = parse_money(parts[4]);
}

std::vector<recurring>& budget::all_recurrings(){
    return recurrings.data;
}
