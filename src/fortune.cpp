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
#include "fortune.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"

using namespace budget;

namespace {

static data_handler<fortune> fortunes;

void list_fortunes(){
    std::vector<std::string> columns = {"ID", "Date", "Amount"};
    std::vector<std::vector<std::string>> contents;

    for(auto& fortune : fortunes.data){
        contents.push_back({to_string(fortune.id), to_string(fortune.check_date), to_string(fortune.amount)});
    }

    display_table(columns, contents);
}

void status_fortunes(){
    std::vector<std::string> columns = {"ID", "Date", "Amount", "Diff.", "Time", "Avg/Day", "Diff. Tot.", "Avg/Day Tot."};
    std::vector<std::vector<std::string>> contents;

    std::vector<budget::fortune> sorted_values = fortunes.data;

    std::sort(sorted_values.begin(), sorted_values.end(),
        [](const budget::fortune& a, const budget::fortune& b){ return a.check_date < b.check_date; });

    budget::money previous;
    budget::money first = sorted_values.front().amount;
    budget::date previous_date;
    budget::date first_date = sorted_values.front().check_date;

    size_t index = 0;

    for(auto& fortune : sorted_values){
        if(index == 0){
            contents.push_back({to_string(fortune.id), to_string(fortune.check_date), to_string(fortune.amount), "", "", "", ""});
        } else if(index == 1){
            auto diff = fortune.amount - previous;
            auto d = fortune.check_date - previous_date;
            auto avg = diff / float(d);
            contents.push_back({to_string(fortune.id), to_string(fortune.check_date), to_string(fortune.amount),
                format_money(diff), to_string(d), to_string(avg), "", ""});
        } else {
            auto diff = fortune.amount - previous;
            auto d = fortune.check_date - previous_date;
            auto avg = diff / float(d);

            auto tot_diff = fortune.amount - first;
            auto tot_d = fortune.check_date - first_date;
            auto tot_avg = tot_diff / float(tot_d);

            contents.push_back({to_string(fortune.id), to_string(fortune.check_date), to_string(fortune.amount),
                format_money(diff), to_string(d), to_string(avg), format_money(tot_diff), to_string(tot_avg)});
        }

        previous = fortune.amount;
        previous_date = fortune.check_date;
        ++index;
    }

    display_table(columns, contents);
}

} //end of anonymous namespace

budget::money budget::current_fortune(){
    auto& all = all_fortunes();

    if(all.empty()){
        return {};
    }

    budget::money fortune_amount = all.front().amount;
    date fortune_date = all.front().check_date;;
    for(auto& fortune : all_fortunes()){
        if(fortune.check_date > fortune_date){
            fortune_amount = fortune.amount;
            fortune_date = fortune.check_date;
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
    if(args.size() == 1){
        status_fortunes();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "list"){
            list_fortunes();
        } else if(subcommand == "status"){
            status_fortunes();
        } else if(subcommand == "check"){
            fortune fortune;
            fortune.guid = generate_guid();
            fortune.check_date = budget::local_day();

            if(args.size() == 2){
                edit_date(fortune.check_date, "Date");

                edit_money(fortune.amount, "Amount");
            } else {
                throw budget_exception("Too many arguments to fortune check");
            }

            auto id = add_data(fortunes, std::move(fortune));
            std::cout << "Fortune check " << id << " has been created" << std::endl;
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if(!exists(fortunes, id)){
                throw budget_exception("There are no fortune with id " + args[2]);
            }

            remove(fortunes, id);

            std::cout << "Fortune check " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if(!exists(fortunes, id)){
                throw budget_exception("There are no fortune with id " + args[2]);
            }

            auto& fortune = get(fortunes, id);

            edit_date(fortune.check_date, "Date");

            edit_money(fortune.amount, "Amount");

            std::cout << "Fortune check " << id << " has been modified" << std::endl;

            fortunes.changed = true;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

std::vector<fortune>& budget::all_fortunes(){
    return fortunes.data;
}

void budget::load_fortunes(){
    load_data(fortunes, "fortunes.data");
}

void budget::save_fortunes(){
    save_data(fortunes, "fortunes.data");
}

std::ostream& budget::operator<<(std::ostream& stream, const fortune& fortune){
    return stream << fortune.id  << ':' << fortune.guid << ':' << to_string(fortune.check_date) << ':' << fortune.amount;
}

void budget::operator>>(const std::vector<std::string>& parts, fortune& fortune){
    fortune.id = to_number<int>(parts[0]);
    fortune.guid = parts[1];
    fortune.check_date = from_string(parts[2]);
    fortune.amount = parse_money(parts[3]);
}

void budget::set_fortunes_changed(){
    fortunes.changed = true;
}

void budget::set_fortunes_next_id(size_t next_id){
    fortunes.next_id = next_id;
}
