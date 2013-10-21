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

#include "args.hpp"
#include "budget_exception.hpp"
#include "fortune.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"

using namespace budget;

namespace {

static data_handler<fortune> fortunes;

void list_fortunes(){
    std::vector<std::string> columns = {"ID", "Date", "Amount"};
    std::vector<std::vector<std::string>> contents;

    for(auto& fortune : fortunes.data){
        contents.push_back({to_string(fortune.id), to_string(fortune.check_date), to_string(fortune.amount)});
    }

    display_table(columns, contents);
}

} //end of anonymous namespace

void budget::fortune_module::load(){
    load_fortunes();
}

void budget::fortune_module::unload(){
    save_fortunes();
}

void budget::fortune_module::handle(const std::vector<std::string>& args){
    if(args.size() == 1){
        list_fortunes();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "list"){
            list_fortunes();
        } else if(subcommand == "check"){
            fortune fortune;
            fortune.guid = generate_guid();
            fortune.check_date = boost::gregorian::day_clock::local_day();

            if(args.size() == 2){
                edit_date(fortune.check_date, "Date");

                edit_money(fortune.amount, "Amount");
            } else {
                throw budget_exception("Too many arguments to fortune check");
            }

            add_data(fortunes, std::move(fortune));
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(fortunes, id)){
                throw budget_exception("There are no fortune with id " + args[2]);
            }

            remove(fortunes, id);

            std::cout << "Fortune " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(fortunes, id)){
                throw budget_exception("There are no fortune with id " + args[2]);
            }

            auto& fortune = get(fortunes, id);

            edit_date(fortune.check_date, "Date");

            edit_money(fortune.amount, "Amount");

            std::cout << "Fortune " << id << " has been modified" << std::endl;

            fortunes.changed = true;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_fortunes(){
    load_data(fortunes, "fortunes.data");
}

void budget::save_fortunes(){
    save_data(fortunes, "fortunes.data");
}

std::ostream& budget::operator<<(std::ostream& stream, const fortune& fortune){
    return stream << fortune.id  << ':' << fortune.guid << ':' << to_string(fortune.check_date) << ':' << fortune.amount;
}

void budget::operator>>(const std::vector<std::string>& parts, fortune& fortune){
    fortune.id = to_number<int>(parts[0]);
    fortune.guid = parts[1];
    fortune.check_date = boost::gregorian::from_string(parts[2]);
    fortune.amount = parse_money(parts[3]);
}
