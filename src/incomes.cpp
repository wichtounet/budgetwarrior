//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "data_cache.hpp"
#include "budget_exception.hpp"
#include "args.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

data_handler<income> incomes{"incomes", "incomes.data"};

} //end of anonymous namespace

std::map<std::string, std::string, std::less<>> budget::income::get_params() const {
    std::map<std::string, std::string, std::less<>> params;

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
        const auto& subcommand = args[1];

        if (subcommand == "show") {
            show_incomes(w);
        } else if (subcommand == "set") {
            budget::money amount;
            edit_money(amount, "Amount", not_negative_checker());

            auto new_income = budget::new_income(amount, true);
            std::cout << "Income " << new_income.id << " has been created" << std::endl;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::show_incomes(budget::writer& w) {
    if (incomes.empty()) {
        w << title_begin << "No income " << set_button("incomes") << title_end;
        return;
    }

    w << title_begin << "Incomes " << set_button("incomes") << title_end;

    w << p_begin << "Current income: " << get_base_income(w.cache) << " " << get_default_currency() << p_end;

    std::vector<std::string> columns = {"ID", "Amount", "Since", "Until", "Edit"};
    std::vector<std::vector<std::string>> contents;

    for (auto& income : w.cache.incomes()) {
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

void budget::income::save(data_writer& writer) const {
    writer << id;
    writer << guid;
    writer << amount;
    writer << since;
    writer << until;
}

void budget::income::load(data_reader& reader) {
    reader >> id;
    reader >> guid;
    reader >> amount;
    reader >> since;
    reader >> until;

    if (config_contains("random")) {
        amount = budget::random_money(1000, 10000);
    }
}

std::vector<income> budget::all_incomes(){
    return incomes.data();
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

income budget::income_get(size_t id) {
    return incomes[id];
}

size_t budget::add_income(budget::income&& income){
    return incomes.add(std::move(income));
}

bool budget::edit_income(const budget::income& income){
    return incomes.indirect_edit(income);
}

budget::money budget::get_base_income(data_cache & cache){
    auto today = budget::local_day();
    return get_base_income(cache, today);
}

budget::money budget::get_base_income(data_cache& cache, const budget::date& d) {
    // First, we try to get the base income from the incomes module

    for (auto & income : cache.incomes()) {
        if (income.since <= d && income.until >= d) {
            return income.amount;
        }
    }

    // Otherwise, we use the accounts

    budget::money income;

    for (auto& account : all_accounts(cache, d.year(), d.month())) {
        income += account.amount;
    }

    return income;
}

budget::income budget::new_income(budget::money amount, bool print){
    budget::date const d = budget::local_day();

    budget::date const since(d.year(), d.month(), 1);
    budget::date const until = budget::date(2099, 12, 31);

    budget::income new_income;
    new_income.guid  = generate_guid();
    new_income.since = since;
    new_income.until = until;
    new_income.amount = amount;

    if (!incomes.empty()) {
        auto incomes_copy = all_incomes();

        // Try to edit the income from the same month
        for (auto & income : incomes_copy) {
            if (income.since == since && income.until == until) {
                income.amount = new_income.amount;

                if (incomes.indirect_edit(income)) {
                    if (print) {
                        std::cout << "Income " << income.id << " has been modified" << std::endl;
                    }
                }

                return income;
            }
        }

        // Edit the previous income

        budget::date date = incomes_copy.front().since;
        size_t id         = incomes_copy.front().id;

        for (auto & income : incomes_copy) {
            if (income.since > date) {
                date = income.since;
                id   = income.id;
            }
        }

        auto previous_income = incomes[id];

        previous_income.until = since - budget::days(1);

        if (incomes.indirect_edit(previous_income)) {
            if (print) {
                std::cout << "Income " << id << " has been modified" << std::endl;
            }
        }
    }

    auto id = incomes.add(std::move(new_income));
    return incomes[id];
}
