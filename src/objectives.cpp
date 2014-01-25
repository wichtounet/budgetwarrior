//=======================================================================
// Copyright (c) 2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "objectives.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "fortune.hpp"
#include "accounts.hpp"
#include "args.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "budget_exception.hpp"

using namespace budget;

namespace {

static data_handler<objective> objectives;

void list_objectives(){
    //TODO
}

void status_objectives(){
    //TODO
}

} //end of anonymous namespace

void budget::objectives_module::load(){
    load_expenses();
    load_earnings();
    load_accounts();
    load_fortunes();
    load_objectives();
}

void budget::objectives_module::unload(){
    save_objectives();
}

void budget::objectives_module::handle(const std::vector<std::string>& args){
    if(args.size() == 1){
        status_objectives();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "list"){
            list_objectives();
        } else if(subcommand == "status"){
            status_objectives();
        } else if(subcommand == "add"){
            //TODO
        } else if(subcommand == "delete"){
            //TODO
        } else if(subcommand == "edit"){
            //TODO
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_objectives(){
    load_data(objectives, "objectives.data");
}

void budget::save_objectives(){
    save_data(objectives, "objectives.data");
}

void budget::add_objective(budget::objective&& objective){
    add_data(objectives, std::forward<budget::objective>(objective));
}

std::ostream& budget::operator<<(std::ostream& stream, const objective& objective){
    return stream
        << objective.id  << ':'
        << objective.guid << ':'
        << objective.name << ':'
        << objective.type << ':'
        << objective.source << ':'
        << objective.op << ':'
        << objective.amount << ':'
        << to_string(objective.date);
}

void budget::operator>>(const std::vector<std::string>& parts, objective& objective){
    objective.id = to_number<std::size_t>(parts[0]);
    objective.guid = parts[1];
    objective.name = parts[2];
    objective.type = parts[3];
    objective.source = parts[4];
    objective.op = parts[5];
    objective.amount = parse_money(parts[6]);
    objective.date = boost::gregorian::from_string(parts[7]);
}

std::vector<objective>& budget::all_objectives(){
    return objectives.data;
}

void budget::set_objectives_changed(){
    objectives.changed = true;
}
