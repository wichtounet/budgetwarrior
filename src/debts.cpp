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
#include "console.hpp"

using namespace budget;

static debts saved_debts;

void budget::list_debts(){
    std::vector<std::string> columns = {"ID", "Direction", "Name", "Amount", "Title"};
    std::vector<std::vector<std::string>> contents;

    money owed;
    money deserved;

    for(auto& debt : saved_debts.debts){
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

int budget::handle_debts(const std::vector<std::string>& args){
    load_debts();

    if(args.size() == 1){
        list_debts();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "list"){
            list_debts();
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

            add_debt(std::move(debt));
        } else if(subcommand == "paid"){
            int id = to_number<int>(args[2]);

            bool found = false;
            for(auto& debt : saved_debts.debts){
                if(debt.id == id){
                    debt.state = 1;

                    found = true;
                    std::cout << "Debt \"" << debt.title << "\" (" << debt.id << ") has been paid" << std::endl;

                    break;
                }
            }

            if(!found){
                std::cout << "There are no debt with id " << id << std::endl;

                return 1;
            }
        } else if(subcommand == "delete"){
            int id = to_number<int>(args[2]);

            std::size_t before = saved_debts.debts.size();
            saved_debts.debts.erase(std::remove_if(saved_debts.debts.begin(), saved_debts.debts.end(), [id](const debt& debt){ return debt.id == id; }), saved_debts.debts.end());

            if(before == saved_debts.debts.size()){
                std::cout << "There are no debt with id " << id << std::endl;

                return 1;
            } else {
                std::cout << "Debt " << id << " has been deleted" << std::endl;
            }
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
                    debt.state = to_number<int>(parts[1]);
                    debt.guid = parts[2];
                    debt.creation_time = boost::posix_time::from_iso_string(parts[3]);
                    debt.direction = to_number<bool>(parts[4]);
                    debt.name = parts[5];
                    debt.amount = parse_money(parts[6]);
                    debt.title = parts[7];

                    saved_debts.debts.push_back(std::move(debt));
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
        file << debt.id  << ':' << debt.state << ':' << debt.guid << ':' << boost::posix_time::to_iso_string(debt.creation_time) << ':' << debt.direction << ':' << debt.name << ':' << debt.amount << ':' << debt.title << std::endl;
    }
}
