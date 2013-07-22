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
#include "earnings.hpp"
#include "expenses.hpp"

using namespace budget;

namespace {

static data_handler<account> accounts;

void show_accounts(){
    std::vector<std::string> columns = {"ID", "Name", "Amount", "Until"};
    std::vector<std::vector<std::string>> contents;

    money total;

    for(auto& account : accounts.data){
        contents.push_back({to_string(account.id), account.name, to_string(account.amount), to_string(account.until)});

        total += account.amount;
    }

    contents.push_back({"", "Total", to_string(total)});

    display_table(columns, contents);
}

} //end of anonymous namespace

void budget::accounts_module::handle(const std::vector<std::string>& args){
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
            account.until = boost::gregorian::date(2099,12,31);

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

            load_expenses();

            for(auto& expense : all_expenses()){
                if(expense.account == id){
                    throw budget_exception("There are still some expenses linked to this account, cannot delete it");
                }
            }

            load_earnings();

            for(auto& earning : all_earnings()){
                if(earning.account == id){
                    throw budget_exception("There are still some earnings linked to this account, cannot delete it");
                }
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

std::ostream& budget::operator<<(std::ostream& stream, const account& account){
    return stream << account.id  << ':' << account.guid << ':' << account.name << ':' << account.amount << ':' << to_string(account.until);
}

void budget::operator>>(const std::vector<std::string>& parts, account& account){
    account.id = to_number<std::size_t>(parts[0]);
    account.guid = parts[1];
    account.name = parts[2];
    account.amount = parse_money(parts[3]);
    account.until = boost::gregorian::from_string(parts[4]);
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
