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

            add_data(accounts, std::move(account));
        } else if(subcommand == "delete"){
            std::size_t id = to_number<std::size_t>(args[2]);

            if(exists(accounts, id)){
                remove(accounts, id);

                std::cout << "Account " << id << " has been deleted" << std::endl;
            } else {
                std::cout << "There are no account with id " << id << std::endl;

                return 1;
            }
        } else {
            std::cout << "Invalid subcommand \"" << subcommand << "\"" << std::endl;

            return 1;
        }
    }

    save_accounts();

    return 0;
}

void budget::load_accounts(){
    load_data(accounts, "accounts.data");
}

void budget::save_accounts(){
    save_data(accounts, "accounts.data");
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

std::ostream& budget::operator<<(std::ostream& stream, const account& account){
    return stream << account.id  << ':' << account.guid << ':' << account.name << ':' << account.amount;
}

void budget::operator>>(const std::vector<std::string>& parts, account& account){
    account.id = to_number<std::size_t>(parts[0]);
    account.guid = parts[1];
    account.name = parts[2];
    account.amount = parse_money(parts[3]);
}

bool budget::account_exists(const std::string& name){
    for(auto& account : accounts.data){
        if(account.name == name){
            return true;
        }
    }

    return false;
}
