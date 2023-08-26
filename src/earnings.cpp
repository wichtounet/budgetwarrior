//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "earnings.hpp"
#include "args.hpp"
#include "accounts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "writer.hpp"
#include "budget_exception.hpp"
#include "views.hpp"

using namespace budget;

namespace {

data_handler<earning> earnings{"earnings", "earnings.data"};

} //end of anonymous namespace

std::map<std::string, std::string> budget::earning::get_params() const {
    std::map<std::string, std::string> params;

    params["input_id"]      = budget::to_string(id);
    params["input_guid"]    = guid;
    params["input_date"]    = budget::to_string(date);
    params["input_name"]    = name;
    params["input_account"] = budget::to_string(account);
    params["input_amount"]  = budget::to_string(amount);

    return params;
}

void budget::earnings_module::load(){
    load_earnings();
    load_accounts();
}

void budget::earnings_module::unload(){
    save_earnings();
}

void budget::earnings_module::handle(const std::vector<std::string>& args){
    console_writer w(std::cout);

    if(args.size() == 1){
        show_earnings(w);
    } else {
        const auto& subcommand = args[1];

        if(subcommand == "show"){
            if(args.size() == 2){
                show_earnings(w);
            } else if(args.size() == 3){
                show_earnings(budget::month(to_number<unsigned short>(args[2])),
                    w);
            } else if(args.size() == 4){
                show_earnings(
                    budget::month(to_number<unsigned short>(args[2])),
                    budget::year(to_number<unsigned short>(args[3])),
                    w);
            } else {
                throw budget_exception("Too many arguments to earning show");
            }
        } else if(subcommand == "all"){
            show_all_earnings(w);
        } else if(subcommand == "add"){
            earning earning;
            earning.guid = generate_guid();
            earning.date = budget::local_day();

            edit_date(earning.date, "Date");

            std::string account_name;
            edit_string_complete(account_name, "Account", all_account_names(), not_empty_checker(), account_checker(earning.date));
            earning.account = get_account(account_name, earning.date.year(), earning.date.month()).id;

            edit_string(earning.name, "Name", not_empty_checker());
            edit_money(earning.amount, "Amount", not_negative_checker());

            auto id = earnings.add(std::move(earning));
            std::cout << "earning " << id << " has been created" << std::endl;
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            const auto id = to_number<size_t>(args[2]);

            if (earnings.remove(id)) {
                std::cout << "earning " << id << " has been deleted" << std::endl;
            } else {
                throw budget_exception("There are no earning with id ");
            }
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            const auto id = to_number<size_t>(args[2]);

            auto earning = earnings[id];

            edit_date(earning.date, "Date");

            auto account_name = get_account(earning.account).name;
            edit_string_complete(account_name, "Account", all_account_names(), not_empty_checker(), account_checker(earning.date));
            earning.account = get_account(account_name, earning.date.year(), earning.date.month()).id;

            edit_string(earning.name, "Name", not_empty_checker());
            edit_money(earning.amount, "Amount", not_negative_checker());

            if (earnings.indirect_edit(earning)) {
                std::cout << "Earning " << id << " has been modified" << std::endl;
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_earnings(){
    earnings.load();
}

void budget::save_earnings(){
    earnings.save();
}

void budget::earning::save(data_writer& writer) const {
    writer << id;
    writer << guid;
    writer << account;
    writer << name;
    writer << amount;
    writer << date;
}

void budget::earning::load(data_reader & reader){
    reader >> id;
    reader >> guid;
    reader >> account;
    reader >> name;
    reader >> amount;
    reader >> date;

    if (config_contains("random")) {
        amount = budget::random_money(10, 5000);
    }
}

std::vector<earning> budget::all_earnings(){
    return earnings.data();
}

bool budget::edit_earning(const earning& earning){
    return earnings.indirect_edit(earning);
}

bool budget::indirect_edit_earning(const earning & earning, bool propagate) {
    return earnings.indirect_edit(earning, propagate);
}

void budget::set_earnings_changed(){
    earnings.set_changed();
}

size_t budget::add_earning(budget::earning&& earning){
    return earnings.add(std::forward<budget::earning>(earning));
}

void budget::show_all_earnings(budget::writer& w){
    w << title_begin << "All Earnings " << add_button("earnings") << title_end;

    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    for(auto& earning : earnings.data()){
        contents.push_back({to_string(earning.id), to_string(earning.date), get_account(earning.account).name, earning.name, to_string(earning.amount)});
    }

    w.display_table(columns, contents);
}

void budget::search_earnings(std::string_view search, budget::writer& w){
    w << title_begin << "Results" << title_end;

    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount", "Edit"};
    std::vector<std::vector<std::string>> contents;

    money total;
    size_t count = 0;

    for(auto& earning : earnings.data()){
        auto it = std::ranges::search(
                earning.name, search, [](char a, char b) { return std::tolower(a) == std::tolower(b); });

        if (it) {
            contents.push_back({to_string(earning.id), to_string(earning.date), get_account(earning.account).name, earning.name, to_string(earning.amount), "::edit::earnings::" + to_string(earning.id)});

            total += earning.amount;
            ++count;
        }
    }

    if(count == 0){
        w << "No earnings found" << end_of_line;
    } else {
        contents.push_back({"", "", "", "Total", to_string(total), ""});

        w.display_table(columns, contents, 1, {}, 0, 1);
    }
}

void budget::show_earnings(budget::month month, budget::year year, budget::writer& w){
    w << title_begin << "Earnings of " << month << " " << year << " "
      << add_button("earnings")
      << budget::year_month_selector{"earnings", year, month} << title_end;

    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount", "Edit"};
    std::vector<std::vector<std::string>> contents;

    money total;
    size_t count = 0;

    for(auto& earning : earnings.data() | filter_by_year(year) | filter_by_month(month)){
        contents.push_back({to_string(earning.id), to_string(earning.date), get_account(earning.account).name, earning.name, to_string(earning.amount), "::edit::earnings::" + to_string(earning.id)});

        total += earning.amount;
        ++count;
    }

    if(count == 0){
        w << "No earnings for " << month << "-" << year << end_of_line;
    } else {
        contents.push_back({"", "", "", "Total", to_string(total), ""});

        w.display_table(columns, contents, 1, {}, 0, 1);
    }
}

void budget::show_earnings(budget::month month, budget::writer& w){
    auto today = budget::local_day();

    budget::show_earnings(month, today.year(), w);
}

void budget::show_earnings(budget::writer& w){
    auto today = budget::local_day();

    budget::show_earnings(today.month(), today.year(), w);
}

bool budget::earning_exists(size_t id){
    return earnings.exists(id);
}

void budget::earning_delete(size_t id) {
    if (!earnings.exists(id)) {
        throw budget_exception("There are no earning with id ");
    }

    earnings.remove(id);
}

earning budget::earning_get(size_t id) {
    return earnings[id];
}
