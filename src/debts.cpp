//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "args.hpp"
#include "budget_exception.hpp"
#include "debts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

static data_handler<debt> debts { "debts", "debts.data" };

void edit_direction(bool& ref, const std::string& title){
    std::string answer;

    std::cout << title << " [" << (ref ? "to" : "from") << "]:";
    std::getline(std::cin, answer);

    if(!answer.empty()){
        auto direction = answer;

        if(direction != "to" && direction != "from"){
            throw budget_exception("Invalid direction, only \"to\" and \"from\" are valid");
        }

        ref = direction == "to" ? true : false;
    }
}

void edit(budget::debt& debt){
    edit_direction(debt.direction, "Direction");
    edit_string(debt.name, "Name", not_empty_checker());
    edit_money(debt.amount, "Amount", not_negative_checker());
    edit_string(debt.title, "Title", not_empty_checker());
}

} //end of anonymous namespace

std::map<std::string, std::string> budget::debt::get_params(){
    std::map<std::string, std::string> params;

    params["input_id"]            = budget::to_string(id);
    params["input_guid"]          = guid;
    params["input_state"]         = budget::to_string(state);
    params["input_creation_date"] = budget::to_string(creation_date);
    params["input_direction"]     = budget::to_string(direction);
    params["input_name"]          = name;
    params["input_amount"]        = budget::to_string(amount);
    params["input_title"]         = title;

    return params;
}

void budget::debt_module::load(){
    load_debts();
}

void budget::debt_module::unload(){
    save_debts();
}

void budget::debt_module::handle(const std::vector<std::string>& args){
    budget::console_writer w(std::cout);

    if(args.size() == 1){
        list_debts(w);
    } else {
        auto& subcommand = args[1];

        if(subcommand == "list"){
            list_debts(w);
        } else if(subcommand == "all"){
            display_all_debts(w);
        } else if(subcommand == "add"){
            debt debt;
            debt.state = 0;
            debt.guid = generate_guid();
            debt.creation_date = local_day();

            edit(debt);

            auto id = add_data(debts, std::move(debt));
            std::cout << "Debt " << id << " has been created" << std::endl;
        } else if(subcommand == "paid"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if(!exists(debts, id)){
                throw budget_exception("There are no debt with id " + args[2]);
            }

            auto& debt = get(debts, id);
            debt.state = 1;

            debts.set_changed();

            std::cout << "Debt \"" << debt.title << "\" (" << debt.id << ") has been paid" << std::endl;
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if(!exists(debts, id)){
                throw budget_exception("There are no debt with id " + args[2]);
            }

            remove(debts, id);

            std::cout << "Debt " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if(!exists(debts, id)){
                throw budget_exception("There are no debt with id " + args[2]);
            }

            edit(get(debts, id));

            std::cout << "Debt " << id << " has been modified" << std::endl;

            debts.set_changed();
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_debts(){
    debts.load();
}

void budget::save_debts(){
    debts.save();
}

std::ostream& budget::operator<<(std::ostream& stream, const debt& debt){
    return stream << debt.id
        << ':' << debt.state
        << ':' << debt.guid
        << ':' << to_string(debt.creation_date)
        << ':' << debt.direction
        << ':' << debt.name
        << ':' << debt.amount
        << ':' << debt.title;
}

void budget::operator>>(const std::vector<std::string>& parts, debt& debt){
    bool random = config_contains("random");

    debt.id = to_number<int>(parts[0]);
    debt.state = to_number<int>(parts[1]);
    debt.guid = parts[2];
    debt.creation_date = from_string(parts[3]);
    debt.direction = to_number<bool>(parts[4]);
    debt.name = parts[5];
    debt.title = parts[7];

    if(random){
        debt.amount = budget::random_money(10, 1000);
    } else {
        debt.amount = parse_money(parts[6]);
    }
}

void budget::migrate_debts_3_to_4(){
    debts.load([](const std::vector<std::string>& parts, debt& debt){
            debt.id = to_number<int>(parts[0]);
            debt.state = to_number<int>(parts[1]);
            debt.guid = parts[2];
            debt.creation_date = from_iso_string(parts[3]);
            debt.direction = to_number<bool>(parts[4]);
            debt.name = parts[5];
            debt.amount = parse_money(parts[6]);
            debt.title = parts[7];
        });

    debts.set_changed();

    debts.save();
}

std::vector<debt>& budget::all_debts(){
    return debts.data;
}

void budget::set_debts_changed(){
    debts.set_changed();
}

void budget::set_debts_next_id(size_t next_id){
    debts.next_id = next_id;
}

void budget::display_all_debts(budget::writer& w){
    std::vector<std::string> columns = {"ID", "Direction", "Name", "Amount", "Paid", "Title", "Edit"};
    std::vector<std::vector<std::string>> contents;

    for(auto& debt : debts.data){
        contents.push_back({to_string(debt.id), debt.direction ? "to" : "from", debt.name, to_string(debt.amount), (debt.state == 0 ? "No" : "Yes"), debt.title, "::edit::debts::" + to_string(debt.id)});
    }

    w.display_table(columns, contents);
}

void budget::list_debts(budget::writer& w){
    std::vector<std::string> columns = {"ID", "Direction", "Name", "Amount", "Title", "Edit"};
    std::vector<std::vector<std::string>> contents;

    money owed;
    money deserved;

    for(auto& debt : debts.data){
        if(debt.state == 0){
            contents.push_back({to_string(debt.id), debt.direction ? "to" : "from", debt.name, to_string(debt.amount), debt.title, "::edit::debts::" + to_string(debt.id)});

            if(debt.direction){
                owed += debt.amount;
            } else {
                deserved += debt.amount;
            }
        }
    }

    contents.push_back({"", "", "", "", "", ""});
    contents.push_back({"", "", "Money owed", budget::to_string(owed), "", ""});
    contents.push_back({"", "", "Money deserved", budget::to_string(deserved), "", ""});

    w.display_table(columns, contents);
}

bool budget::debt_exists(size_t id){
    return exists(debts, id);
}

void budget::debt_delete(size_t id) {
    if (!exists(debts, id)) {
        throw budget_exception("There are no debt with id ");
    }

    remove(debts, id);
}

debt& budget::debt_get(size_t id) {
    if (!exists(debts, id)) {
        throw budget_exception("There are no debt with id ");
    }

    return get(debts, id);
}

void budget::add_debt(budget::debt&& debt){
    add_data(debts, std::forward<budget::debt>(debt));
}
