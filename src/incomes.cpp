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
        show_incomes(w);
    } else {
        auto& subcommand = args[1];

        if (subcommand == "show") {
            show_incomes(w);
        } else if (subcommand == "set") {
            budget::date d = budget::local_day();

            budget::date since(d.year(), d.month(), 1);
            budget::date until = budget::date(2099,12,31);

            budget::income new_income;
            new_income.guid  = generate_guid();
            new_income.since = since;
            new_income.until = until;

            edit_money(new_income.amount, "Amount", not_negative_checker());

            if (incomes.size()) {
                // Try to edit the income from the same month
                for (auto & income : incomes) {
                    if (income.since == since && income.until == until) {
                        income.amount = new_income.amount;

                        if (incomes.edit(income)) {
                            std::cout << "Income " << income.id << " has been modified" << std::endl;
                        }

                        return;
                    }
                }

                // Edit the previous income

                budget::date date = incomes.data.front().since;
                size_t id         = incomes.data.front().id;

                for (auto & income : incomes) {
                    if (income.since > date) {
                        date = income.since;
                        id   = income.id;
                    }
                }

                auto & previous_income = incomes[id];

                previous_income.until = since - budget::days(1);

                if (incomes.edit(previous_income)) {
                    std::cout << "Income " << id << " has been modified" << std::endl;
                }
            }

            auto id = incomes.add(std::move(new_income));
            std::cout << "Income " << id << " has been created" << std::endl;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::show_incomes(budget::writer& w) {
    if (!incomes.size()) {
        w << title_begin << "No income " << add_button("incomes") << title_end;
        return;
    }

    w << title_begin << "Incomes " << add_button("incomes") << title_end;

    std::vector<std::string> columns = {"ID", "Amount", "Since", "Until", "Edit"};
    std::vector<std::vector<std::string>> contents;

    for(auto& income : incomes.data){
        contents.push_back({to_string(income.id), to_string(income.amount), to_string(income.since), to_string(income.until), "::edit::incomes::" + to_string(income.id)});
    }

    w.display_table(columns, contents);
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
