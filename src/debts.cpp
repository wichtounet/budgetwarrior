//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "debts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "money.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"

using namespace budget;

static data_handler<debt> debts;

int budget::handle_debts(const std::vector<std::string>& args){
    load_debts();

    if(args.size() == 1){
        list_debts();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "list"){
            list_debts();
        } else if(subcommand == "all"){
            all_debts();
        } else if(subcommand == "add"){
            if(args.size() < 5){
                std::cout << "Not enough args for debt add" << std::endl;

                return 1;
            }

            debt debt;
            debt.state = 0;
            debt.guid = generate_guid();
            debt.creation_time = boost::posix_time::second_clock::local_time();

            std::string direction = args[2];

            if(direction != "to" && direction != "from"){
                std::cout << "Invalid direction, only \"to\" and \"from\" are valid" << std::endl;

                return 1;
            }

            debt.direction = direction == "to" ? true : false;

            debt.name = args[3];

            std::string amount_string = args[4];
            debt.amount = parse_money(amount_string);

            if(debt.amount.dollars < 0 || debt.amount.cents < 0){
                std::cout << "Amount of the debt cannot be negative" << std::endl;

                return 1;
            }

            if(args.size() > 5){
                for(std::size_t i = 5; i < args.size(); ++i){
                    debt.title += args[i] + " ";
                }
            }

            add_data(debts, std::move(debt));
        } else if(subcommand == "paid"){
            std::size_t id = to_number<std::size_t>(args[2]);

            if(exists(debts, id)){
                for(auto& debt : debts.data){
                    if(debt.id == id){
                        debt.state = 1;

                        std::cout << "Debt \"" << debt.title << "\" (" << debt.id << ") has been paid" << std::endl;

                        break;
                    }
                }
            } else {
                std::cout << "There are no debt with id " << id << std::endl;

                return 1;
            }
        } else if(subcommand == "delete"){
            std::size_t id = to_number<std::size_t>(args[2]);

            if(exists(debts, id)){
                remove(debts, id);

                std::cout << "Debt " << id << " has been deleted" << std::endl;
            } else {
                std::cout << "There are no debt with id " << id << std::endl;

                return 1;
            }
        } else {
            std::cout << "Invalid subcommand \"" << subcommand << "\"" << std::endl;

            return 1;
        }
    }

    save_debts();

    return 0;
}

void budget::load_debts(){
    load_data(debts, "debts.data");
}

void budget::save_debts(){
    save_data(debts, "debts.data");
}

void budget::all_debts(){
    std::vector<std::string> columns = {"ID", "Direction", "Name", "Amount", "Paid", "Title"};
    std::vector<std::vector<std::string>> contents;

    for(auto& debt : debts.data){
        contents.push_back({to_string(debt.id), debt.direction ? "to" : "from", debt.name, to_string(debt.amount), (debt.state == 0 ? "No" : "Yes"), debt.title});
    }

    display_table(columns, contents);
}

void budget::list_debts(){
    std::vector<std::string> columns = {"ID", "Direction", "Name", "Amount", "Title"};
    std::vector<std::vector<std::string>> contents;

    money owed;
    money deserved;

    for(auto& debt : debts.data){
        if(debt.state == 0){
            contents.push_back({to_string(debt.id), debt.direction ? "to" : "from", debt.name, to_string(debt.amount), debt.title});

            if(debt.direction){
                owed += debt.amount;
            } else {
                deserved += debt.amount;
            }
        }
    }

    display_table(columns, contents);
    std::cout << std::endl;

    std::cout << std::string(7, ' ') << "Money owed: " << owed << std::endl;
    std::cout << std::string(3, ' ') << "Money deserved: " << deserved << std::endl;
}

std::ostream& budget::operator<<(std::ostream& stream, const debt& debt){
    return stream << debt.id  << ':' << debt.state << ':' << debt.guid << ':' << boost::posix_time::to_iso_string(debt.creation_time) << ':' << debt.direction << ':' << debt.name << ':' << debt.amount << ':' << debt.title;
}

void budget::operator>>(const std::vector<std::string>& parts, debt& debt){
    debt.id = to_number<int>(parts[0]);
    debt.state = to_number<int>(parts[1]);
    debt.guid = parts[2];
    debt.creation_time = boost::posix_time::from_iso_string(parts[3]);
    debt.direction = to_number<bool>(parts[4]);
    debt.name = parts[5];
    debt.amount = parse_money(parts[6]);
    debt.title = parts[7];
}
