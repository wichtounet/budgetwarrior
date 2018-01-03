//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
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

static data_handler<wish> wishes;

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

} //end of anonymous namespace

void budget::wishes_module::load(){
    load_expenses();
    load_earnings();
    load_accounts();
    load_fortunes();
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

            auto id = add_data(wishes, std::move(wish));
            std::cout << "Wish " << id << " has been created" << std::endl;
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if(!exists(wishes, id)){
                throw budget_exception("There are no wish with id " + args[2]);
            }

            remove(wishes, id);

            std::cout << "wish " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if(!exists(wishes, id)){
                throw budget_exception("There are no wish with id " + args[2]);
            }

            auto& wish = get(wishes, id);

            edit(wish);

            set_wishes_changed();

            std::cout << "wish " << id << " has been modified" << std::endl;
        } else if(subcommand == "paid"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if(!exists(wishes, id)){
                throw budget_exception("There are no wish with id " + args[2]);
            }

            auto& wish = get(wishes, id);

            edit_money(wish.paid_amount, "Paid Amount", not_negative_checker(), not_zero_checker());

            wish.paid = true;

            set_wishes_changed();

            std::cout << "wish " << id << " has been marked as paid" << std::endl;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_wishes(){
    load_data(wishes, "wishes.data");
}

void budget::save_wishes(){
    save_data(wishes, "wishes.data");
}

std::ostream& budget::operator<<(std::ostream& stream, const wish& wish){
    return stream
        << wish.id  << ':'
        << wish.guid << ':'
        << wish.name << ':'
        << wish.amount << ':'
        << to_string(wish.date) << ':'
        << static_cast<size_t>(wish.paid) << ':'
        << wish.paid_amount << ':'
        << wish.importance << ':'
        << wish.urgency;
}

void budget::operator>>(const std::vector<std::string>& parts, wish& wish){
    bool random = config_contains("random");

    wish.id = to_number<size_t>(parts[0]);
    wish.guid = parts[1];
    wish.name = parts[2];
    wish.date = from_string(parts[4]);
    wish.paid = to_number<size_t>(parts[5]) == 1;
    wish.importance = to_number<size_t>(parts[7]);
    wish.urgency = to_number<size_t>(parts[8]);

    if(random){
        wish.amount = budget::random_money(100, 1000);
        wish.paid_amount = budget::random_money(100, 1000);
    } else {
        wish.amount = parse_money(parts[3]);
        wish.paid_amount = parse_money(parts[6]);
    }
}

std::vector<wish>& budget::all_wishes(){
    return wishes.data;
}

void budget::set_wishes_changed(){
    wishes.set_changed();
}

void budget::set_wishes_next_id(size_t next_id){
    wishes.next_id = next_id;
}

void budget::migrate_wishes_2_to_3(){
    load_data(wishes, "wishes.data", [](const std::vector<std::string>& parts, wish& wish){
        wish.id = to_number<size_t>(parts[0]);
        wish.guid = parts[1];
        wish.name = parts[2];
        wish.amount = parse_money(parts[3]);
        wish.date = from_string(parts[4]);
        wish.paid = false;
        wish.paid_amount = budget::money(0,0);
        });

    set_wishes_changed();

    save_data(wishes, "wishes.data");
}

void budget::migrate_wishes_3_to_4(){
    load_data(wishes, "wishes.data", [](const std::vector<std::string>& parts, wish& wish){
        wish.id = to_number<size_t>(parts[0]);
        wish.guid = parts[1];
        wish.name = parts[2];
        wish.amount = parse_money(parts[3]);
        wish.date = from_string(parts[4]);
        wish.paid = to_number<size_t>(parts[5]) == 1;
        wish.paid_amount = parse_money(parts[6]);
        wish.importance = 2;
        wish.urgency = 2;
        });

    set_wishes_changed();

    save_data(wishes, "wishes.data");
}

void budget::list_wishes(budget::writer& w){
    w << title_begin << "Wishes" << title_end;

    if (wishes.data.size() == 0) {
        w << "No wishes" << end_of_line;
    } else {
        std::vector<std::string> columns = {"ID", "Name", "Importance", "Urgency", "Amount", "Paid", "Diff", "Accuracy", "Edit"};
        std::vector<std::vector<std::string>> contents;

        money total;
        money unpaid_total;
        double acc         = 0.0;
        double acc_counter = 0;

        for (auto& wish : wishes.data) {
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

        w.display_table(columns, contents);
    }
}

void budget::status_wishes(budget::writer& w){
    w << title_begin << "Wishes" << title_end;

    auto today = budget::local_day();

    if(today.day() < 12){
        w << "WARNING: It is early in the month, no one can know what may happen ;)" << end_of_line;
    }

    std::vector<std::string> columns = {"ID", "Name", "Amount", "I", "U", "Status", "Details", "Edit"};
    std::vector<std::vector<std::string>> contents;

    auto month_status = budget::compute_month_status(today.year(), today.month());
    auto year_status = budget::compute_year_status(today.year(), today.month());

    auto fortune_amount = budget::current_fortune();

    budget::money total_amount;

    for(auto& wish : wishes.data){
        if(wish.paid){
            continue;
        }

        total_amount += wish.amount;

        size_t monthly_breaks = 0;
        size_t yearly_breaks = 0;

        bool month_objective = true;
        bool year_objective = true;

        for(auto& objective : all_objectives()){
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
                if(!all_objectives().empty()){
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
                if(!all_objectives().empty()){
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

    w.display_table(columns, contents);
}

void budget::estimate_wishes(budget::writer& w) {
    std::vector<std::string> columns = {"ID", "Name", "Amount", "Status", "Edit"};
    std::vector<std::vector<std::string>> year_contents;
    std::vector<std::vector<std::string>> month_contents;

    auto fortune_amount = budget::current_fortune();
    auto today          = budget::local_day();

    for (auto& wish : wishes.data) {
        if (wish.paid) {
            continue;
        }

        bool ok = false;

        std::string status;

        for (size_t i = 0; i < 24 && !ok; ++i) {
            auto day = today + months(i);

            auto month_status = budget::compute_month_status(day.year(), day.month());
            auto year_status  = budget::compute_year_status(day.year(), day.month());

            size_t monthly_breaks = 0;
            size_t yearly_breaks  = 0;

            bool month_objective = true;
            bool year_objective  = true;

            for (auto& objective : all_objectives()) {
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

    for (auto& wish : wishes.data) {
        if (wish.paid) {
            continue;
        }

        bool ok = false;

        std::string status;

        for (size_t i = 0; i < 24 && !ok; ++i) {
            auto day          = today + months(i);
            auto month_status = budget::compute_month_status(day.year(), day.month());
            auto year_status  = budget::compute_year_status(day.year(), day.month());

            size_t monthly_breaks = 0;
            bool month_objective  = true;

            for (auto& objective : all_objectives()) {
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
    return exists(wishes, id);
}

void budget::wish_delete(size_t id) {
    if (!exists(wishes, id)) {
        throw budget_exception("There are no wish with id ");
    }

    remove(wishes, id);
}

wish& budget::wish_get(size_t id) {
    if (!exists(wishes, id)) {
        throw budget_exception("There are no wish with id ");
    }

    return get(wishes, id);
}

void budget::add_wish(budget::wish&& wish){
    add_data(wishes, std::forward<budget::wish>(wish));
}
