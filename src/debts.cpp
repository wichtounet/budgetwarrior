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
#include "debts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"

using namespace budget;

static data_handler<debt> debts;

void budget::handle_debts(const std::vector<std::wstring>& args){
    load_debts();

    if(args.size() == 1){
        list_debts();
    } else {
        auto& subcommand = args[1];

        if(subcommand == L"list"){
            list_debts();
        } else if(subcommand == L"all"){
            all_debts();
        } else if(subcommand == L"add"){
            enough_args(args, 5);

            debt debt;
            debt.state = 0;
            debt.guid = generate_guid();
            debt.creation_time = boost::posix_time::second_clock::local_time();

            std::wstring direction = args[2];

            if(direction != L"to" && direction != L"from"){
                throw budget_exception(L"Invalid direction, only \"to\" and \"from\" are valid");
            }

            debt.direction = direction == L"to" ? true : false;

            debt.name = args[3];

            debt.amount = parse_money(args[4]);
            not_negative(debt.amount);

            if(args.size() > 5){
                for(std::size_t i = 5; i < args.size(); ++i){
                    debt.title += args[i] + L" ";
                }
            }

            add_data(debts, std::move(debt));
        } else if(subcommand == L"paid"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(debts, id)){
                throw budget_exception(L"There are no debt with id " + args[2]);
            }

            for(auto& debt : debts.data){
                if(debt.id == id){
                    debt.state = 1;

                    std::wcout << L"Debt \"" << debt.title << L"\" (" << debt.id << L") has been paid" << std::endl;

                    break;
                }
            }
        } else if(subcommand == L"delete"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(debts, id)){
                throw budget_exception(L"There are no debt with id " + args[2]);
            }

            remove(debts, id);

            std::wcout << L"Debt " << id << L" has been deleted" << std::endl;
        } else {
            throw budget_exception(L"Invalid subcommand \"" + subcommand + L"\"");
        }
    }

    save_debts();
}

void budget::load_debts(){
    load_data(debts, "debts.data");
}

void budget::save_debts(){
    save_data(debts, "debts.data");
}

void budget::all_debts(){
    std::vector<std::wstring> columns = {L"ID", L"Direction", L"Name", L"Amount", L"Paid", L"Title"};
    std::vector<std::vector<std::wstring>> contents;

    for(auto& debt : debts.data){
        contents.push_back({to_string(debt.id), debt.direction ? L"to" : L"from", debt.name, to_string(debt.amount), (debt.state == 0 ? L"No" : L"Yes"), debt.title});
    }

    display_table(columns, contents);
}

void budget::list_debts(){
    std::vector<std::wstring> columns = {L"ID", L"Direction", L"Name", L"Amount", L"Title"};
    std::vector<std::vector<std::wstring>> contents;

    money owed;
    money deserved;

    for(auto& debt : debts.data){
        if(debt.state == 0){
            contents.push_back({to_string(debt.id), debt.direction ? L"to" : L"from", debt.name, to_string(debt.amount), debt.title});

            if(debt.direction){
                owed += debt.amount;
            } else {
                deserved += debt.amount;
            }
        }
    }

    display_table(columns, contents);
    std::wcout << std::endl;

    std::wcout << std::wstring(7, ' ') << L"Money owed: " << owed << std::endl;
    std::wcout << std::wstring(3, ' ') << L"Money deserved: " << deserved << std::endl;
}

std::wostream& budget::operator<<(std::wostream& stream, const debt& debt){
    return stream << debt.id  << ':' << debt.state << ':' << debt.guid << ':' << to_wstring(boost::posix_time::to_iso_string(debt.creation_time)) << ':' << debt.direction << ':' << debt.name << ':' << debt.amount << ':' << debt.title;
}

void budget::operator>>(const std::vector<std::wstring>& parts, debt& debt){
    debt.id = to_number<int>(parts[0]);
    debt.state = to_number<int>(parts[1]);
    debt.guid = parts[2];
    debt.creation_time = boost::posix_time::from_iso_string(to_nstring(parts[3]));
    debt.direction = to_number<bool>(parts[4]);
    debt.name = parts[5];
    debt.amount = parse_money(parts[6]);
    debt.title = parts[7];
}
