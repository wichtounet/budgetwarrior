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

#include "wishes.hpp"
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

static data_handler<wish> wishes;

void list_wishes(){
    if(wishes.data.size() == 0){
        std::cout << "No wishes" << std::endl;
    } else {
        std::vector<std::string> columns = {"ID", "Name", "Amount"};
        std::vector<std::vector<std::string>> contents;

        for(auto& wish : wishes.data){
            contents.push_back({to_string(wish.id), wish.name, to_string(wish.amount)});
        }

        display_table(columns, contents);
    }
}

void status_wishes(){
    //TODO
}

void edit(budget::wish& wish){
    edit_string(wish.name, "Name");
    not_empty(wish.name, "The name of the wish cannot be empty");

    edit_money(wish.amount, "Amount");
}

} //end of anonymous namespace

void budget::wishes_module::load(){
    load_expenses();
    load_earnings();
    load_accounts();
    load_fortunes();
    load_objectives();
    load_wishes();
}

void budget::wishes_module::unload(){
    save_wishes();
}

void budget::wishes_module::handle(const std::vector<std::string>& args){
    if(args.size() == 1){
        status_wishes();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "list"){
            list_wishes();
        } else if(subcommand == "status"){
            status_wishes();
        } else if(subcommand == "add"){
            wish wish;
            wish.guid = generate_guid();
            wish.date = boost::gregorian::day_clock::local_day();

            edit(wish);

            add_data(wishes, std::move(wish));
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(wishes, id)){
                throw budget_exception("There are no wish with id ");
            }

            remove(wishes, id);

            std::cout << "wish " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(wishes, id)){
                throw budget_exception("There are no wish with id " + args[2]);
            }

            auto& wish = get(wishes, id);

            edit(wish);

            set_wishes_changed();
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_wishes(){
    load_data(wishes, "wishes.data");
}

void budget::save_wishes(){
    save_data(wishes, "wishes.data");
}

void budget::add_wish(budget::wish&& wish){
    add_data(wishes, std::forward<budget::wish>(wish));
}

std::ostream& budget::operator<<(std::ostream& stream, const wish& wish){
    return stream
        << wish.id  << ':'
        << wish.guid << ':'
        << wish.name << ':'
        << wish.amount << ':'
        << to_string(wish.date);
}

void budget::operator>>(const std::vector<std::string>& parts, wish& wish){
    wish.id = to_number<std::size_t>(parts[0]);
    wish.guid = parts[1];
    wish.name = parts[2];
    wish.amount = parse_money(parts[3]);
    wish.date = boost::gregorian::from_string(parts[4]);
}

std::vector<wish>& budget::all_wishes(){
    return wishes.data;
}

void budget::set_wishes_changed(){
    wishes.changed = true;
}
