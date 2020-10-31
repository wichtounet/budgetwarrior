//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
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

std::map<std::string, std::string> budget::debt::get_params() const {
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

            auto id = debts.add(std::move(debt));
            std::cout << "Debt " << id << " has been created" << std::endl;
        } else if(subcommand == "paid"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            auto debt = debts[id];
            debt.state = 1;

            if (debts.indirect_edit(debt)) {
                std::cout << "Debt \"" << debt.title << "\" (" << debt.id << ") has been paid" << std::endl;
            }
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if (debts.remove(id)) {
                std::cout << "Debt " << id << " has been deleted" << std::endl;
            } else {
                throw budget_exception("There are no debt with id " + args[2]);
            }
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            auto debt = debts[id];
            edit(debt);

            if (debts.indirect_edit(debt)) {
                std::cout << "Debt " << id << " has been modified" << std::endl;
            }
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

void budget::debt::save(data_writer & writer){
    writer << id;
    writer << state;
    writer << guid;
    writer << creation_date;
    writer << direction;
    writer << name;
    writer << amount;
    writer << title;
}

void budget::debt::load(data_reader & reader){
    reader >> id;
    reader >> state;
    reader >> guid;
    reader >> creation_date;
    reader >> direction;
    reader >> name;
    reader >> amount;
    reader >> title;

    if (config_contains("random")) {
        amount = budget::random_money(10, 1000);
    }
}

std::vector<debt> budget::all_debts(){
    return debts.data();
}

void budget::set_debts_changed(){
    debts.set_changed();
}

void budget::set_debts_next_id(size_t next_id){
    debts.next_id = next_id;
}

void budget::display_all_debts(budget::writer& w){
    w << title_begin << "All debts " << add_button("debts") << title_end;

    std::vector<std::string> columns = {"ID", "Direction", "Name", "Amount", "Paid", "Title", "Edit"};
    std::vector<std::vector<std::string>> contents;

    for(auto& debt : debts.data()){
        contents.push_back({to_string(debt.id), debt.direction ? "to" : "from", debt.name, to_string(debt.amount), (debt.state == 0 ? "No" : "Yes"), debt.title, "::edit::debts::" + to_string(debt.id)});
    }

    w.display_table(columns, contents);
}

void budget::list_debts(budget::writer& w){
    w << title_begin << "Debts " << add_button("debts") << title_end;

    bool found = false;
    for (auto& debt : debts.data()) {
        if (debt.state == 0) {
            found = true;
            break;
        }
    }

    if(!found){
        w << "No active debt! Well done!" << end_of_line;
    } else {
        std::vector<std::string> columns = {"ID", "Direction", "Name", "Amount", "Title", "Edit"};
        std::vector<std::vector<std::string>> contents;

        money owed;
        money deserved;

        for (auto& debt : debts.data()) {
            if (debt.state == 0) {
                contents.push_back({to_string(debt.id), debt.direction ? "to" : "from", debt.name, to_string(debt.amount), debt.title, "::edit::debts::" + to_string(debt.id)});

                if (debt.direction) {
                    owed += debt.amount;
                } else {
                    deserved += debt.amount;
                }
            }
        }

        contents.push_back({"", "", "", "", "", ""});
        contents.push_back({"", "", "Money owed", budget::to_string(owed), "", ""});
        contents.push_back({"", "", "Money deserved", budget::to_string(deserved), "", ""});

        w.display_table(columns, contents, 1, {}, 0, 3);
    }
}

bool budget::debt_exists(size_t id){
    return debts.exists(id);
}

void budget::debt_delete(size_t id) {
    if (!debts.exists(id)) {
        throw budget_exception("There are no debt with id ");
    }

    debts.remove(id);
}

debt budget::debt_get(size_t id) {
    return debts[id];
}

void budget::edit_debt(const budget::debt& debt){
    debts.indirect_edit(debt);
}

void budget::add_debt(budget::debt&& debt){
    debts.add(std::forward<budget::debt>(debt));
}
