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
#include "budget_exception.hpp"
#include "args.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "money.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"

using namespace budget;

static data_handler<account> accounts;

void budget::handle_accounts(const std::vector<std::string>& args){
    load_accounts();

    if(args.size() == 1){
        show_accounts();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "show"){
            show_accounts();
        } else if(subcommand == "add"){
            enough_args(args, 4);

            account account;
            account.guid = generate_guid();
            account.name = args[2];

            if(account_exists(account.name)){
                throw budget_exception("An account with this name already exists");
            }

            account.amount = parse_money(args[3]);
            not_negative(account.amount);

            add_data(accounts, std::move(account));
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(accounts, id)){
                throw budget_exception("There are no account with id " + args[2]);
            }

            remove(accounts, id);

            std::cout << "Account " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(accounts, id)){
                throw budget_exception("There are no account with id " + args[2]);
            }

            auto& account = get(accounts, id);

            edit_string(account.name, "Name");
            edit_money(account.amount, "Amount");

            std::cout << "Account " << id << " has been modified" << std::endl;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }

    save_accounts();
}

void budget::load_accounts(){
    load_data(accounts, "accounts.data");
}

void budget::save_accounts(){
    save_data(accounts, "accounts.data");
}

budget::account& budget::get_account(std::size_t id){
    return get(accounts, id);
}

budget::account& budget::get_account(std::string name){
    for(auto& account : accounts.data){
        if(account.name == name){
            return account;
        }
    }

    budget_unreachable("The account does not exist");
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

std::vector<account>& budget::all_accounts(){
    return accounts.data;
}
