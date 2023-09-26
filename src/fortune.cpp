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
#include "fortune.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

data_handler<fortune> fortunes{"fortunes", "fortunes.data"};

} //end of anonymous namespace

std::map<std::string, std::string, std::less<>> budget::fortune::get_params() const {
    std::map<std::string, std::string, std::less<>> params;

    params["input_id"]          = budget::to_string(id);
    params["input_guid"]        = guid;
    params["input_check_date"]  = budget::to_string(check_date);
    params["input_amount"]      = budget::to_string(amount);

    return params;
}

void budget::list_fortunes(budget::writer& w){
    if (fortunes.empty()) {
        w << "No fortune set" << end_of_line;
        return;
    }

    std::vector<std::string> columns = {"ID", "Date", "Amount", "Edit"};
    std::vector<std::vector<std::string>> contents;

    for (auto& fortune : fortunes.data()) {
        contents.push_back({to_string(fortune.id), to_string(fortune.check_date), to_string(fortune.amount), "::edit::fortunes::" + budget::to_string(fortune.id)});
    }

    w.display_table(columns, contents);
}

void budget::status_fortunes(budget::writer& w, bool short_view){
    if(fortunes.empty()){
        w << "No fortune set" << end_of_line;
        return;
    }

    const std::vector<std::string> short_columns = {"From", "To", "Amount", "Diff.", "Avg/Day", "Avg/Day Tot."};
    const std::vector<std::string> long_columns = {"From",    "To",         "Amount",       "Diff.", "Time",
                                                   "Avg/Day", "Diff. Tot.", "Avg/Day Tot.", "Edit"};

    auto columns = short_view ? short_columns : long_columns;
    std::vector<std::vector<std::string>> contents;

    auto sorted_values = fortunes.data();

    std::ranges::sort(sorted_values, [](const budget::fortune& a, const budget::fortune& b) { return a.check_date < b.check_date; });

    budget::money previous;
    budget::money const first = sorted_values.front().amount;
    budget::date previous_date{};
    budget::date const first_date = sorted_values.front().check_date;

    for(std::size_t i = 0; i < sorted_values.size(); ++i){
        auto& fortune = sorted_values[i];

        bool display = false;
        if(!short_view){
            display = true;
        } else {
            const size_t displayed = 3;
            if(sorted_values.size() <= displayed){
                display = true;
            } else {
                display = i > sorted_values.size() - 1 - displayed;
            }
        }

        if (display) {
            if (i == 0) {
                if (short_view) {
                    contents.push_back({"", to_string(fortune.check_date), to_string(fortune.amount), "", "", ""});
                } else {
                    contents.push_back({"", to_string(fortune.check_date), to_string(fortune.amount), "", "", "", "", "", "::edit::fortunes::" + budget::to_string(fortune.id)});
                }
            } else if (i == 1) {
                const money diff = fortune.amount - previous;
                const long d = fortune.check_date - previous_date;
                const money avg = diff / d;

                if (short_view) {
                    contents.push_back({to_string(previous_date), to_string(fortune.check_date), to_string(fortune.amount),
                                        format_money(diff), to_string(avg), ""});
                } else {
                    contents.push_back({to_string(previous_date), to_string(fortune.check_date), to_string(fortune.amount),
                                        format_money(diff), to_string(d), to_string(avg), "", "", "::edit::fortunes::" + budget::to_string(fortune.id)});
                }
            } else {
                const money diff = fortune.amount - previous;
                const long d = fortune.check_date - previous_date;
                const money avg = diff / d;

                const money tot_diff = fortune.amount - first;
                const long tot_d = fortune.check_date - first_date;
                const money tot_avg = tot_diff / tot_d;

                if (short_view) {
                    contents.push_back({to_string(previous_date), to_string(fortune.check_date), to_string(fortune.amount),
                                        format_money(diff), to_string(avg), to_string(tot_avg)});
                } else {
                    contents.push_back({to_string(previous_date), to_string(fortune.check_date), to_string(fortune.amount),
                                        format_money(diff), to_string(d), to_string(avg), format_money(tot_diff), to_string(tot_avg), "::edit::fortunes::" + budget::to_string(fortune.id)});
                }
            }
        }

        previous = fortune.amount;
        previous_date = fortune.check_date;
    }

    w.display_table(columns, contents);
}

budget::money budget::current_fortune(){
    auto all = all_fortunes();

    if (all.empty()) {
        return {};
    }

    budget::money fortune_amount = all.front().amount;
    date fortune_date            = all.front().check_date;

    for (auto& fortune : all) {
        if (fortune.check_date > fortune_date) {
            fortune_amount = fortune.amount;
            fortune_date   = fortune.check_date;
        }
    }

    return fortune_amount;
}

void budget::fortune_module::load(){
    load_fortunes();
}

void budget::fortune_module::unload(){
    save_fortunes();
}

void budget::fortune_module::handle(const std::vector<std::string>& args){
    console_writer w(std::cout);

    if(args.size() == 1){
        return status_fortunes(w, false);
    }

    const auto& subcommand = args[1];

    if (subcommand == "list") {
        list_fortunes(w);
    } else if (subcommand == "status") {
        status_fortunes(w, false);
    } else if (subcommand == "check") {
        fortune fortune;
        fortune.guid       = generate_guid();
        fortune.check_date = budget::local_day();

        if (args.size() == 2) {
            edit_date(fortune.check_date, "Date");

            edit_money(fortune.amount, "Amount");
        } else {
            throw budget_exception("Too many arguments to fortune check");
        }

        auto id = fortunes.add(std::move(fortune));
        std::cout << "Fortune check " << id << " has been created" << std::endl;
    } else if (subcommand == "delete") {
        enough_args(args, 3);

        const auto id = to_number<size_t>(args[2]);

        if (!fortunes.exists(id)) {
            throw budget_exception("There are no fortune with id " + args[2]);
        }

        fortunes.remove(id);

        std::cout << "Fortune check " << id << " has been deleted" << std::endl;
    } else if (subcommand == "edit") {
        enough_args(args, 3);

        const auto id = to_number<size_t>(args[2]);

        auto fortune = fortunes[id];

        edit_date(fortune.check_date, "Date");

        edit_money(fortune.amount, "Amount");

        if (fortunes.indirect_edit(fortune)) {
            std::cout << "Fortune check " << id << " has been modified" << std::endl;
        }
    } else {
        throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
    }
}

std::vector<fortune> budget::all_fortunes(){
    return fortunes.data();
}

void budget::load_fortunes(){
    fortunes.load();
}

void budget::save_fortunes(){
    fortunes.save();
}

void budget::fortune::save(data_writer& writer) const {
    writer << id;
    writer << guid;
    writer << check_date;
    writer << amount;
}

void budget::fortune::load(data_reader & reader){
    reader >> id;
    reader >> guid;
    reader >> check_date;
    reader >> amount;

    if (config_contains("random")) {
        amount = budget::random_money(1000, 100000);
    }
}

void budget::set_fortunes_changed(){
    fortunes.set_changed();
}

void budget::set_fortunes_next_id(size_t next_id){
    fortunes.next_id = next_id;
}

bool budget::fortune_exists(size_t id){
    return fortunes.exists(id);
}

void budget::fortune_delete(size_t id) {
    if (!fortunes.exists(id)) {
        throw budget_exception("There are no fortune with id ");
    }

    fortunes.remove(id);
}

fortune budget::fortune_get(size_t id) {
    return fortunes[id];
}

bool budget::no_fortunes() {
    return fortunes.empty();
}

void budget::edit_fortune(const budget::fortune& fortune){
    fortunes.indirect_edit(fortune);
}

size_t budget::add_fortune(budget::fortune&& fortune){
    return fortunes.add(std::move(fortune));
}
