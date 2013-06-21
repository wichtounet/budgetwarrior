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
#include "guid.hpp"
#include "money.hpp"
#include "config.hpp"
#include "utils.hpp"

using namespace budget;

static debts saved_debts;

int budget::handle_debts(const std::vector<std::string>& args){
    load_debts();

    if(args.size() == 1){
        std::cout << "List of debts" << std::endl;

        //TODO Implement display
    } else {
        auto& subcommand = args[1];

        if(subcommand == "add"){
            if(args.size() < 5){
                std::cout << "Not enough args for debt add" << std::endl;

                return 1;
            }

            debt debt;
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
                    debt.title += args[i];
                }
            }

            add_debt(std::move(debt));
        } else {
            std::cout << "Invalid subcommand \"" << subcommand << "\"" << std::endl;

            return 1;
        }
    }

    save_debts();

    return 0;
}

void budget::add_debt(budget::debt&& debt){
    debt.id = saved_debts.next_id++;

    saved_debts.debts.push_back(std::forward<budget::debt>(debt));
}

void budget::load_debts(){
    auto file_path = path_to_budget_file("debts.data");

    if(!boost::filesystem::exists(file_path)){
        saved_debts.next_id = 1;
    } else {
        std::ifstream file(file_path);

        if(file.is_open()){
            if(file.good()){
                file >> saved_debts.next_id;
                file.get();

                std::string line;
                while(file.good() && getline(file, line)){
                    std::vector<std::string> parts;
                    boost::split(parts, line, boost::is_any_of(":"), boost::token_compress_on);

                    debt debt;
                    debt.id = to_number<int>(parts[0]);
                    debt.guid = parts[1];
                    debt.creation_time = boost::posix_time::from_iso_string(parts[2]);
                    debt.direction = to_number<bool>(parts[3]);
                    debt.name = parts[4];
                    debt.amount = parse_money(parts[5]);
                    debt.title = parts[6];

                    add_debt(std::move(debt));
                }
            }
        }
    }
}

void budget::save_debts(){
    auto file_path = path_to_budget_file("debts.data");

    std::ofstream file(file_path);
    file << saved_debts.next_id << std::endl;

    for(auto& debt : saved_debts.debts){
        file << debt.id  << ':' << debt.guid << ':' << boost::posix_time::to_iso_string(debt.creation_time) << ':' << debt.direction << ':' << debt.name << ':' << debt.amount << ':' << debt.title << std::endl;
    }
}
