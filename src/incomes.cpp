//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "incomes.hpp"
#include "accounts.hpp"
#include "budget_exception.hpp"
#include "args.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "earnings.hpp"
#include "expenses.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

static data_handler<income> incomes { "incomes", "incomes.data" };

} //end of anonymous namespace

std::map<std::string, std::string> budget::income::get_params(){
    std::map<std::string, std::string> params;

    params["input_id"]     = budget::to_string(id);
    params["input_guid"]   = guid;
    params["input_amount"] = budget::to_string(amount);
    params["input_since"]  = budget::to_string(since);
    params["input_until"]  = budget::to_string(until);

    return params;
}

void budget::incomes_module::load(){
    load_incomes();
}

void budget::incomes_module::unload(){
    save_incomes();
}

void budget::incomes_module::handle(const std::vector<std::string>& args){
    console_writer w(std::cout);

    if(args.size() == 1){
        // Show current income
    } else {
        auto& subcommand = args[1];

        if (subcommand == "set") {
            // TODO
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_incomes(){
    incomes.load();
}

void budget::save_incomes(){
    incomes.save();
}

std::ostream& budget::operator<<(std::ostream& stream, const income& income){
    return stream << income.id  << ':' << income.guid << ':' << income.amount << ':' << to_string(income.since) << ':' << to_string(income.until);
}

void budget::operator>>(const std::vector<std::string>& parts, income& income){
    bool random = config_contains("random");

    income.id = to_number<size_t>(parts[0]);
    income.guid = parts[1];
    income.since = from_string(parts[3]);
    income.until = from_string(parts[4]);

    if(random){
        income.amount = budget::random_money(1000, 10000);
    } else {
        income.amount = parse_money(parts[2]);
    }
}

std::vector<income>& budget::all_incomes(){
    return incomes.data;
}

void budget::set_incomes_changed(){
    incomes.set_changed();
}

void budget::set_incomes_next_id(size_t next_id){
    incomes.next_id = next_id;
}

bool budget::income_exists(size_t id){
    return incomes.exists(id);
}

void budget::income_delete(size_t id) {
    if (!incomes.exists(id)) {
        throw budget_exception("There are no income with id ");
    }

    incomes.remove(id);
}

income& budget::income_get(size_t id) {
    if (!incomes.exists(id)) {
        throw budget_exception("There are no income with id ");
    }

    return incomes[id];
}

void budget::add_income(budget::income&& income){
    incomes.add(std::forward<budget::income>(income));
}
