//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "cpp_utils/string.hpp"

#include "wishes.hpp"
#include "objectives.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "fortune.hpp"
#include "assets.hpp"
#include "accounts.hpp"
#include "args.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "budget_exception.hpp"
#include "compute.hpp"
#include "console.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

static data_handler<wish> wishes { "wishes", "wishes.data" };

std::string wish_status(size_t v){
    switch(v){
        case 1:
            return "::greenLow";
        case 2:
            return "Medium";
        case 3:
            return "::redHigh";
        default:
            cpp_unreachable("Invalid status value");
            return "::redInvalid";
    }
}

std::string wish_status_short(size_t v){
    switch(v){
        case 1:
            return "::greenL";
        case 2:
            return "M";
        case 3:
            return "::redH";
        default:
            cpp_unreachable("Invalid status value");
            return red("Invalid");
    }
}

std::string accuracy(budget::money paid, budget::money estimation){
    auto a = 100.0 * (static_cast<double>(paid.dollars()) / static_cast<double>(estimation.dollars()));

    return to_string(static_cast<size_t>(a)) + "%";
}

void edit(budget::wish& wish){
    edit_string(wish.name, "Name", not_empty_checker());
    edit_money(wish.amount, "Amount", not_negative_checker(), not_zero_checker());
    edit_number(wish.importance, "Importance", range_checker<1,3>());
    edit_number(wish.urgency, "Urgency", range_checker<1,3>());
}

budget::money cash_for_wishes(){
    if(budget::net_worth_over_fortune()){
        return get_net_worth_cash();
    } else {
        return current_fortune();
    }
}

} //end of anonymous namespace

std::map<std::string, std::string> budget::wish::get_params() const {
    std::map<std::string, std::string> params;

    params["input_id"]          = budget::to_string(id);
    params["input_guid"]        = guid;
    params["input_name"]        = name;
    params["input_amount"]      = budget::to_string(amount);
    params["input_paid"]        = paid ? "true" : "false";
    params["input_paid_amount"] = budget::to_string(paid_amount);
    params["input_importance"]  = budget::to_string(importance);
    params["input_urgency"]     = budget::to_string(urgency);

    return params;
}

void budget::wishes_module::load(){
    load_expenses();
    load_earnings();
    load_accounts();

    // Need to load both to make sure to have the correct information
    load_assets();
    load_fortunes();

    // Need to be loaded last
    load_objectives();
    load_wishes();
}

void budget::wishes_module::unload(){
    save_wishes();
}

void budget::wishes_module::handle(const std::vector<std::string>& args){
    console_writer w(std::cout);

    if(args.size() == 1){
        status_wishes(w);
    } else {
        auto& subcommand = args[1];

        if(subcommand == "list"){
            list_wishes(w);
        } else if(subcommand == "status"){
            status_wishes(w);
        } else if(subcommand == "estimate"){
            estimate_wishes(w);
        } else if(subcommand == "add"){
            wish wish;
            wish.guid = generate_guid();
            wish.date = budget::local_day();
            wish.importance = 2;
            wish.urgency = 2;

            edit(wish);

            auto id = wishes.add(std::move(wish));
            std::cout << "Wish " << id << " has been created" << std::endl;
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if (wishes.remove(id)) {
                std::cout << "wish " << id << " has been deleted" << std::endl;
            } else {
                throw budget_exception("There are no wish with id " + args[2]);
            }
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            auto wish = wishes[id];

            edit(wish);

            if (wishes.indirect_edit(wish)) {
                std::cout << "Wish " << id << " has been modified" << std::endl;
            }
        } else if(subcommand == "paid"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            auto wish = wishes[id];

            edit_money(wish.paid_amount, "Paid Amount", not_negative_checker(), not_zero_checker());

            wish.paid = true;

            if (wishes.indirect_edit(wish)) {
                std::cout << "Wish " << id << " has been marked as paid" << std::endl;
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_wishes(){
    wishes.load();
}

void budget::save_wishes(){
    wishes.save();
}

void budget::wish::save(data_writer & writer){
    writer << id;
    writer << guid;
    writer << name;
    writer << amount;
    writer << date;
    writer << paid;
    writer << paid_amount;
    writer << importance;
    writer << urgency;
}

void budget::wish::load(data_reader & reader){
    reader >> id;
    reader >> guid;
    reader >> name;
    reader >> amount;
    reader >> date;
    reader >> paid;
    reader >> paid_amount;
    reader >> importance;
    reader >> urgency;

    if (config_contains("random")) {
        amount      = budget::random_money(100, 1000);
        paid_amount = budget::random_money(100, 1000);
    }
}

std::vector<wish> budget::all_wishes(){
    return wishes.data();
}

void budget::set_wishes_changed(){
    wishes.set_changed();
}

void budget::set_wishes_next_id(size_t next_id){
    wishes.next_id = next_id;
}

void budget::list_wishes(budget::writer& w){
    w << title_begin << "Wishes " << add_button("wishes") << title_end;

    if (wishes.size() == 0) {
        w << "No wishes" << end_of_line;
    } else {
        std::vector<std::string> columns = {"ID", "Name", "Importance", "Urgency", "Amount", "Paid", "Diff", "Accuracy", "Edit"};
        std::vector<std::vector<std::string>> contents;

        money total;
        money unpaid_total;
        double acc         = 0.0;
        double acc_counter = 0;

        for (auto& wish : wishes.data()) {
            contents.push_back({to_string(wish.id), wish.name, wish_status(wish.importance), wish_status(wish.urgency),
                                to_string(wish.amount),
                                wish.paid ? to_string(wish.paid_amount) : "No",
                                wish.paid ? format_money_reverse(wish.paid_amount - wish.amount) : "",
                                wish.paid ? accuracy(wish.paid_amount, wish.amount) : "", "::edit::wishes::" + to_string(wish.id)});

            total += wish.amount;

            if (wish.paid) {
                auto a = static_cast<double>(wish.paid_amount.dollars()) / wish.amount.dollars();

                acc += a;
                acc_counter += 1;
            } else {
                unpaid_total += wish.amount;
            }
        }

        contents.push_back({"", "", "", "", "", "", "", "", ""});
        contents.push_back({"", "Total", "", "", to_string(total), "", "", "", ""});
        contents.push_back({"", "Unpaid Total", "", "", to_string(unpaid_total), "", "", "", ""});

        if (acc_counter > 0) {
            contents.push_back({"", "Mean accuracy", "", "", to_string(static_cast<size_t>((acc / acc_counter) * 100.0)) + "%", "", "", "", ""});
        }

        w.display_table(columns, contents, 1, {}, 0, acc_counter > 0 ? 4 : 3);
    }
}

void budget::status_wishes(budget::writer& w){
    w << title_begin << "Wishes " << add_button("wishes") << title_end;

    auto today = budget::local_day();

    if(today.day() < 12){
        w << "WARNING: It is early in the month, no one can know what may happen ;)" << end_of_line;
    }

    std::vector<std::string> columns = {"ID", "Name", "Amount", "I", "U", "Status", "Details", "Edit"};
    std::vector<std::vector<std::string>> contents;

    auto month_status = budget::compute_month_status(w.cache, today.year(), today.month());
    auto year_status = budget::compute_year_status(w.cache, today.year(), today.month());

    auto fortune_amount = cash_for_wishes();

    budget::money total_amount;

    for(auto& wish : w.cache.wishes()){
        if(wish.paid){
            continue;
        }

        total_amount += wish.amount;

        size_t monthly_breaks = 0;
        size_t yearly_breaks = 0;

        bool month_objective = true;
        bool year_objective = true;

        for(auto& objective : w.cache.objectives()){
            if(objective.type == "monthly"){
                auto success_before = budget::compute_success(month_status, objective);
                auto success_after = budget::compute_success(month_status.add_expense(wish.amount), objective);

                if(success_before >= 100 && success_after < 100){
                    ++monthly_breaks;
                }

                if(success_after < 100){
                    month_objective = false;
                }
            } else if(objective.type == "yearly"){
                auto success_before = budget::compute_success(year_status, objective);
                auto success_after = budget::compute_success(year_status.add_expense(wish.amount), objective);

                if(success_before >= 100 && success_after < 100){
                    ++yearly_breaks;
                }

                if(success_after < 100){
                    year_objective = false;
                }
            }
        }

        std::string status;
        std::string details;

        if(fortune_amount < wish.amount){
            status = std::string("::red") + "Impossible";
            details = "(not enough fortune)";
        } else {
            if(month_status.balance > wish.amount){
                if(!w.cache.objectives().empty()){
                    if(month_objective && year_objective){
                        status = std::string("::green") + "Perfect";
                        details = "(On month balance, all objectives fullfilled)";
                    } else if(month_objective){
                        status = std::string("::green") + "Good";
                        details = "(On month balance, month objectives fullfilled)";
                    } else if(yearly_breaks > 0 || monthly_breaks > 0){
                        status = "::blueOK";
                        details = "(On month balance, " + to_string(yearly_breaks + monthly_breaks) + " objectives broken)";
                    } else if(yearly_breaks == 0 && monthly_breaks == 0){
                        status = std::string("::red") + "Warning";
                        details = "(On month balance, objectives not fullfilled)";
                    }
                } else {
                    status = "OK (on month balance)";
                    details = "(on month balance)";
                }
            } else if(year_status.balance > wish.amount){
                if(!w.cache.objectives().empty()){
                    if(month_objective && year_objective){
                        status = std::string("::green") + "Perfect";
                        details = "(On year balance, all objectives fullfilled)";
                    } else if(month_objective){
                        status = std::string("::green") + "Good";
                        details = "(On year balance, month objectives fullfilled)";
                    } else if(yearly_breaks > 0 || monthly_breaks > 0){
                        status = "::blueOK";
                        details = "(On year balance, " + to_string(yearly_breaks + monthly_breaks) + " objectives broken)";
                    } else if(yearly_breaks == 0 && monthly_breaks == 0){
                        status = std::string("::red") + "Warning";
                        details = "(On year balance, objectives not fullfilled)";
                    }
                } else {
                    status = "::blueOK";
                    details = "(on year balance)";
                }
            } else {
                status = std::string("::red") + "Warning";
                details = "(on fortune only)";
            }
        }

        contents.push_back({to_string(wish.id), wish.name, to_string(wish.amount), wish_status_short(wish.importance), wish_status_short(wish.urgency), status, details, "::edit::wishes::" + to_string(wish.id)});
    }

    contents.push_back({"", "", "", "", "", "", "", ""});
    contents.push_back({"", "Total", to_string(total_amount), "", "", "", "", ""});

    w.display_table(columns, contents, 1, {}, 0, 2);
}

void budget::estimate_wishes(budget::writer& w) {
    std::vector<std::string> columns = {"ID", "Name", "Amount", "Status", "Edit"};
    std::vector<std::vector<std::string>> year_contents;
    std::vector<std::vector<std::string>> month_contents;

    auto fortune_amount = cash_for_wishes();
    auto today          = budget::local_day();

    for (auto& wish : w.cache.wishes()) {
        if (wish.paid) {
            continue;
        }

        bool ok = false;

        std::string status;

        for (size_t i = 0; i < 24 && !ok; ++i) {
            auto day = today + months(i);

            auto month_status = budget::compute_month_status(w.cache, day.year(), day.month());
            auto year_status  = budget::compute_year_status(w.cache, day.year(), day.month());

            size_t monthly_breaks = 0;
            size_t yearly_breaks  = 0;

            bool month_objective = true;
            bool year_objective  = true;

            for (auto& objective : w.cache.objectives()) {
                if (objective.type == "monthly") {
                    auto success_before = budget::compute_success(month_status, objective);
                    auto success_after  = budget::compute_success(month_status.add_expense(wish.amount), objective);

                    if (success_before >= 100 && success_after < 100) {
                        ++monthly_breaks;
                    }

                    if (success_after < 100) {
                        month_objective = false;
                    }
                } else if (objective.type == "yearly") {
                    auto success_before = budget::compute_success(year_status, objective);
                    auto success_after  = budget::compute_success(year_status.add_expense(wish.amount), objective);

                    if (success_before >= 100 && success_after < 100) {
                        ++yearly_breaks;
                    }

                    if (success_after < 100) {
                        year_objective = false;
                    }
                }
            }

            if (fortune_amount >= wish.amount) {
                if (month_objective && year_objective) {
                    if (wish.amount >= month_status.budget) {
                        if (year_status.balance > wish.amount) {
                            status = day.month().as_short_string() + " " + to_string(day.year());
                            ok     = true;
                        }
                    } else {
                        if (month_status.balance > wish.amount) {
                            status = day.month().as_short_string() + " " + to_string(day.year());
                            ok     = true;
                        }
                    }
                }
            }
        }

        if (!ok) {
            status = "You should wait until next year to buy this";
        }

        year_contents.push_back({to_string(wish.id), wish.name, to_string(wish.amount), status, "::edit::wishes::" + to_string(wish.id)});
    }

    for (auto& wish : w.cache.wishes()) {
        if (wish.paid) {
            continue;
        }

        bool ok = false;

        std::string status;

        for (size_t i = 0; i < 24 && !ok; ++i) {
            auto day          = today + months(i);
            auto month_status = budget::compute_month_status(w.cache, day.year(), day.month());
            auto year_status  = budget::compute_year_status(w.cache, day.year(), day.month());

            size_t monthly_breaks = 0;
            bool month_objective  = true;

            for (auto& objective : w.cache.objectives()) {
                if (objective.type == "monthly") {
                    auto success_before = budget::compute_success(month_status, objective);
                    auto success_after  = budget::compute_success(month_status.add_expense(wish.amount), objective);

                    if (success_before >= 100 && success_after < 100) {
                        ++monthly_breaks;
                    }

                    if (success_after < 100) {
                        month_objective = false;
                    }
                }
            }

            if (fortune_amount >= wish.amount) {
                if (month_objective) {
                    if (wish.amount >= month_status.budget) {
                        if (year_status.balance > wish.amount) {
                            status = day.month().as_short_string() + " " + to_string(day.year());
                            ok     = true;
                        }
                    } else {
                        if (month_status.balance > wish.amount) {
                            status = day.month().as_short_string() + " " + to_string(day.year());
                            ok     = true;
                        }
                    }
                }
            }
        }

        if (!ok) {
            status = "You should wait a very long time to buy this";
        }

        month_contents.push_back({to_string(wish.id), wish.name, to_string(wish.amount), status, "::edit::wishes::" + to_string(wish.id)});
    }

    w << title_begin << "Time to buy (with year objectives)" << title_end;
    w.display_table(columns, year_contents);

    w << title_begin << "Time to buy (without year objectives)" << title_end;
    w.display_table(columns, month_contents);
}

bool budget::wish_exists(size_t id){
    return wishes.exists(id);
}

void budget::wish_delete(size_t id) {
    if (!wishes.exists(id)) {
        throw budget_exception("There are no wish with id ");
    }

    wishes.remove(id);
}

wish budget::wish_get(size_t id) {
    return wishes[id];
}

void budget::add_wish(budget::wish&& wish){
    wishes.add(std::forward<budget::wish>(wish));
}

bool budget::edit_wish(const budget::wish& wish){
    return wishes.indirect_edit(wish);
}
