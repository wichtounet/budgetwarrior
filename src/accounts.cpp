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

#include "accounts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "money.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"

using namespace budget;

static data_handler<account> accounts;

int budget::handle_accounts(const std::vector<std::string>& args){
    load_accounts();

    if(args.size() == 1){
        show_accounts();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "show"){
            show_accounts();
        } else if(subcommand == "add"){
            if(args.size() < 4){
                std::cout << "Not enough args for account add" << std::endl;

                return 1;
            }

            account account;
            account.guid = generate_guid();
            account.name = args[2];

            std::string amount_string = args[3];
            account.amount = parse_money(amount_string);

            if(account.amount.dollars < 0 || account.amount.cents < 0){
                std::cout << "Amount of the account cannot be negative" << std::endl;

                return 1;
            }

            add_account(std::move(account));
        } else if(subcommand == "delete"){
            std::size_t id = to_number<std::size_t>(args[2]);

            std::size_t before = accounts.data.size();
            accounts.data.erase(std::remove_if(accounts.data.begin(), accounts.data.end(), [id](const account& account){ return account.id == id; }), accounts.data.end());

            if(before == accounts.data.size()){
                std::cout << "There are no account with id " << id << std::endl;

                return 1;
            } else {
                std::cout << "Account " << id << " has been deleted" << std::endl;
            }
        } else {
            std::cout << "Invalid subcommand \"" << subcommand << "\"" << std::endl;

            return 1;
        }
    }

    save_accounts();

    return 0;
}

void budget::add_account(budget::account&& account){
    account.id = accounts.next_id++;

    accounts.data.push_back(std::forward<budget::account>(account));
}

void budget::load_accounts(){
    auto file_path = path_to_budget_file("accounts.data");

    if(!boost::filesystem::exists(file_path)){
        accounts.next_id = 1;
    } else {
        std::ifstream file(file_path);

        if(file.is_open()){
            if(file.good()){
                file >> accounts.next_id;
                file.get();

                std::string line;
                while(file.good() && getline(file, line)){
                    std::vector<std::string> parts;
                    boost::split(parts, line, boost::is_any_of(":"), boost::token_compress_on);

                    account account;
                    account.id = to_number<std::size_t>(parts[0]);
                    account.guid = parts[1];
                    account.name = parts[2];
                    account.amount = parse_money(parts[3]);

                    accounts.data.push_back(std::move(account));
                }
            }
        }
    }
}

void budget::save_accounts(){
    auto file_path = path_to_budget_file("accounts.data");

    std::ofstream file(file_path);
    file << accounts.next_id << std::endl;

    for(auto& account : accounts.data){
        file << account.id  << ':' << account.guid << ':' << account.name << ':' << account.amount << std::endl;
    }
}

void budget::show_accounts(){
    std::vector<std::string> columns = {"ID", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    money total;

    for(auto& account : accounts.data){
        contents.push_back({to_string(account.id), account.name, to_string(account.amount)});

        total += account.amount;
    }

    contents.push_back({"", "Total", to_string(total)});

    display_table(columns, contents);
}
