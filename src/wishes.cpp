//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

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

using namespace budget;

namespace {

static data_handler<wish> wishes;

std::string status(std::size_t v){
    switch(v){
        case 1:
            return "::greenLow";
        case 2:
            return "Medium";
        case 3:
            return "::redHigh";
        default:
            budget_unreachable("Invalid status value");
            return "::redInvalid";
    }
}

std::string status_short(std::size_t v){
    switch(v){
        case 1:
            return green("L");
        case 2:
            return "M";
        case 3:
            return red("H");
        default:
            budget_unreachable("Invalid status value");
            return red("Invalid");
    }
}

void list_wishes(){
    if(wishes.data.size() == 0){
        std::cout << "No wishes" << std::endl;
    } else {
        std::vector<std::string> columns = {"ID", "Name", "Importance", "Urgency", "Amount", "Paid", "Diff"};
        std::vector<std::vector<std::string>> contents;

        money total;
        money unpaid_total;
        for(auto& wish : wishes.data){
            contents.push_back({
                to_string(wish.id), wish.name, status(wish.importance), status(wish.urgency), 
                to_string(wish.amount), 
                wish.paid ? to_string(wish.paid_amount) : "No", 
                wish.paid ? format_money_reverse(wish.paid_amount - wish.amount) : ""});

            total += wish.amount;
            if(!wish.paid){
                unpaid_total += wish.amount;
            }
        }

        contents.push_back({"", "", "", ""});
        contents.push_back({"", "Total", to_string(total), ""});
        contents.push_back({"", "Unpaid Total", to_string(unpaid_total), ""});

        display_table(columns, contents);
    }
}

void status_wishes(){
    auto today = budget::local_day();

    if(today.day() < 12){
        std::cout << "WARNING: It is early in the month, no one can know what may happen ;)" << std::endl << std::endl;
    }

    std::cout << "Wishes" << std::endl << std::endl;

    size_t width = 0;
    for(auto& wish : wishes.data){
        if(wish.paid){
            continue;
        }

        auto name = wish.name + " (" + to_string(wish.amount) + ")";
        trim(name);

        width = std::max(rsize(name), width);
    }

    auto month_status = budget::compute_month_status(today.year(), today.month());
    auto year_status = budget::compute_year_status(today.year(), today.month());

    auto fortune_amount = budget::current_fortune();

    budget::money total_amount;

    for(auto& wish : wishes.data){
        if(wish.paid){
            continue;
        }

        auto amount = wish.amount;
        auto name = wish.name + " (" + to_string(wish.amount) + ")";
        
        trim(name);

        std::cout << "  ";
        print_minimum(name, width);
        
        std::cout << "  ";
        std::cout << status_short(wish.importance) << "-" << status_short(wish.urgency);

        std::cout << "  ";

        total_amount += amount;

        size_t monthly_breaks = 0;
        size_t yearly_breaks = 0;

        bool month_objective = true;
        bool year_objective = true;

        for(auto& objective : all_objectives()){
            if(objective.type == "monthly"){
                auto success_before = budget::compute_success(month_status, objective);
                auto success_after = budget::compute_success(month_status.add_expense(amount), objective);

                if(success_before >= 100 && success_after < 100){
                    ++monthly_breaks;
                }

                if(success_after < 100){
                    month_objective = false;
                }
            } else if(objective.type == "yearly"){
                auto success_before = budget::compute_success(year_status, objective);
                auto success_after = budget::compute_success(year_status.add_expense(amount), objective);

                if(success_before >= 100 && success_after < 100){
                    ++yearly_breaks;
                }

                if(success_after < 100){
                    year_objective = false;
                }
            }
        }

        if(fortune_amount < wish.amount){
            std::cout << red("Impossible") << " (not enough fortune)";
        } else {
            if(month_status.balance > wish.amount){
                if(!all_objectives().empty()){
                    if(month_objective && year_objective){
                        std::cout << green("Perfect") << " (On month balance, all objectives fullfilled)";
                    } else if(month_objective){
                        std::cout << green("Good") << " (On month balance, month objectives fullfilled)";
                    } else if(yearly_breaks > 0 || monthly_breaks > 0){
                        std::cout << cyan("OK") << " (On month balance, " << (yearly_breaks + monthly_breaks) << " objectives broken)";
                    } else if(yearly_breaks == 0 && monthly_breaks == 0){
                        std::cout << red("Warning") << " (On month balance, objectives not fullfilled)";
                    }
                } else {
                    std::cout << "OK (on month balance)";
                }
            } else if(year_status.balance > wish.amount){
                if(!all_objectives().empty()){
                    if(month_objective && year_objective){
                        std::cout << green("Perfect") << " (On year balance, all objectives fullfilled)";
                    } else if(month_objective){
                        std::cout << green("Good") << " (On year balance, month objectives fullfilled)";
                    } else if(yearly_breaks > 0 || monthly_breaks > 0){
                        std::cout << cyan("OK") << " (On year balance, " << (yearly_breaks + monthly_breaks) << " objectives broken)";
                    } else if(yearly_breaks == 0 && monthly_breaks == 0){
                        std::cout << red("Warning") << " (On year balance, objectives not fullfilled)";
                    }
                } else {
                    std::cout << cyan("OK") << " (on year balance)";
                }
            } else {
                std::cout << red("Warning") << " (on fortune only)";
            }
        }

        std::cout << std::endl;
    }
        
    std::cout << std::endl << "  ";
    print_minimum("Total", width);
    std::cout << "  " << total_amount << std::endl;
}

void estimate_wishes(){
    std::cout << "Time to buy (with year objectives)" << std::endl;

    size_t width = 0;
    for(auto& wish : wishes.data){
        if(wish.paid){
            continue;
        }
        
        auto name = wish.name + " (" + to_string(wish.amount) + ")";
        
        trim(name);

        width = std::max(rsize(name), width);
    }

    auto fortune_amount = budget::current_fortune();
    auto today = budget::local_day();

    for(auto& wish : wishes.data){
        if(wish.paid){
            continue;
        }
        
        auto name = wish.name + " (" + to_string(wish.amount) + ")";
        
        trim(name);

        std::cout << "  ";
        print_minimum(name, width);
        std::cout << "  ";

        bool ok = false;
    
        for(std::size_t i = 0; i < 24 && !ok; ++i){
            auto day = today + boost::gregorian::months(i);
            auto month_status = budget::compute_month_status(day.year(), day.month());
            auto year_status = budget::compute_year_status(day.year(), day.month());

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

            if(fortune_amount >= wish.amount){
                if(month_objective && year_objective){
                    if(wish.amount >= month_status.budget){
                        if(year_status.balance > wish.amount){
                            std::cout << day.month() << " " << day.year() << std::endl;
                            ok = true;
                        } 
                    } else {
                        if(month_status.balance > wish.amount){
                            std::cout << day.month() << " " << day.year() << std::endl;
                            ok = true;
                        }
                    }
                } 
            }
        }

        if(!ok){
            std::cout << "You should wait a very long time to buy this" << std::endl;
        }
    }
    
    std::cout << std::endl << "Time to buy (without year objectives)" << std::endl;
    
    for(auto& wish : wishes.data){
        if(wish.paid){
            continue;
        }

        auto name = wish.name + " (" + to_string(wish.amount) + ")";
        
        trim(name);

        std::cout << "  ";
        print_minimum(name, width);
        std::cout << "  ";

        bool ok = false;
    
        for(std::size_t i = 0; i < 24 && !ok; ++i){
            auto day = today + boost::gregorian::months(i);
            auto month_status = budget::compute_month_status(day.year(), day.month());
            auto year_status = budget::compute_year_status(day.year(), day.month());

            size_t monthly_breaks = 0;
            bool month_objective = true;

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
                } 
            }

            if(fortune_amount >= wish.amount){
                if(month_objective){
                    if(wish.amount >= month_status.budget){
                        if(year_status.balance > wish.amount){
                            std::cout << day.month() << " " << day.year() << std::endl;
                            ok = true;
                        } 
                    } else {
                        if(month_status.balance > wish.amount){
                            std::cout << day.month() << " " << day.year() << std::endl;
                            ok = true;
                        }
                    }
                } 
            }
        }

        if(!ok){
            std::cout << "You should wait a very long time to buy this" << std::endl;
        }
    }
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
    if(args.size() == 1){
        status_wishes();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "list"){
            list_wishes();
        } else if(subcommand == "status"){
            status_wishes();
        } else if(subcommand == "estimate"){
            estimate_wishes();
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

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(wishes, id)){
                throw budget_exception("There are no wish with id " + args[2]);
            }

            remove(wishes, id);

            std::cout << "wish " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

            if(!exists(wishes, id)){
                throw budget_exception("There are no wish with id " + args[2]);
            }

            auto& wish = get(wishes, id);

            edit(wish);

            set_wishes_changed();

            std::cout << "wish " << id << " has been modified" << std::endl;
        } else if(subcommand == "paid"){
            enough_args(args, 3);

            std::size_t id = to_number<std::size_t>(args[2]);

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

void budget::add_wish(budget::wish&& wish){
    add_data(wishes, std::forward<budget::wish>(wish));
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
    wish.id = to_number<std::size_t>(parts[0]);
    wish.guid = parts[1];
    wish.name = parts[2];
    wish.amount = parse_money(parts[3]);
    wish.date = from_string(parts[4]);
    wish.paid = to_number<std::size_t>(parts[5]) == 1;
    wish.paid_amount = parse_money(parts[6]);
    wish.importance = to_number<std::size_t>(parts[7]);
    wish.urgency = to_number<std::size_t>(parts[8]);
}

std::vector<wish>& budget::all_wishes(){
    return wishes.data;
}

void budget::set_wishes_changed(){
    wishes.changed = true;
}

void budget::migrate_wishes_2_to_3(){
    load_data(wishes, "wishes.data", [](const std::vector<std::string>& parts, wish& wish){
        wish.id = to_number<std::size_t>(parts[0]);
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
        wish.id = to_number<std::size_t>(parts[0]);
        wish.guid = parts[1];
        wish.name = parts[2];
        wish.amount = parse_money(parts[3]);
        wish.date = from_string(parts[4]);
        wish.paid = to_number<std::size_t>(parts[5]) == 1;
        wish.paid_amount = parse_money(parts[6]);
        wish.importance = 2;
        wish.urgency = 2;
        });

    set_wishes_changed();

    save_data(wishes, "wishes.data");
}
