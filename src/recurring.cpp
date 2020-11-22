//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "recurring.hpp"
#include "args.hpp"
#include "accounts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "budget_exception.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

static data_handler<recurring> recurrings { "recurrings", "recurrings.data" };

budget::date last_date(const budget::recurring& recurring) {
    budget::date last(1400, 1, 1);

    if (recurring.type == "expense") {
        for (auto& expense : all_expenses()) {
            if (expense.name == recurring.name && expense.amount == recurring.amount && get_account(expense.account).name == recurring.account) {
                if (expense.date > last) {
                    last = expense.date;
                }
            }
        }
    } else if (recurring.type == "earning") {
        for (auto& earning : all_earnings()) {
            if (earning.name == recurring.name && earning.amount == recurring.amount && get_account(earning.account).name == recurring.account) {
                if (earning.date > last) {
                    last = earning.date;
                }
            }
        }
    } else {
        throw budget_exception("Invalid recurring type " + recurring.type);
    }

    return last;
}

bool recurring_not_triggered(const budget::recurring & recurring ) {
    return last_date(recurring).year() == 1400;
}

void generate_recurring(budget::date date, const recurring & recurring) {
    if (recurring.type == "expense") {
        budget::expense recurring_expense;

        recurring_expense.guid    = generate_guid();
        recurring_expense.date    = date;
        recurring_expense.account = get_account(recurring.account, date.year(), date.month()).id;
        recurring_expense.amount  = recurring.amount;
        recurring_expense.name    = recurring.name;

        add_expense(std::move(recurring_expense));
    } else if (recurring.type == "earning") {
        budget::earning recurring_earning;

        recurring_earning.guid    = generate_guid();
        recurring_earning.date    = date;
        recurring_earning.account = get_account(recurring.account, date.year(), date.month()).id;
        recurring_earning.amount  = recurring.amount;
        recurring_earning.name    = recurring.name;

        add_earning(std::move(recurring_earning));
    } else {
        throw budget_exception("Invalid recurring type " + recurring.type);
    }
}

} //end of anonymous namespace

std::map<std::string, std::string> budget::recurring::get_params()  const {
    std::map<std::string, std::string> params;

    params["input_id"]      = budget::to_string(id);
    params["input_guid"]    = guid;
    params["input_name"]    = name;
    params["input_amount"]  = budget::to_string(amount);
    params["input_recurs"]  = recurs;
    params["input_account"] = account;
    params["input_type"]    = type;

    return params;
}

void budget::check_for_recurrings(){
    // In random mode, we do not try to create recurrings
    if (config_contains("random")) {
        return;
    }

    auto now = budget::local_day();

    bool changed = false;

    for (auto& recurring : recurrings.data()) {
        if (recurring.recurs == "monthly") {
            if (recurring_not_triggered(recurring)) {
                // If the recurring has never been created, we create it for
                // the first at the time of today

                generate_recurring({now.year(), now.month(), 1}, recurring);

                changed = true;
            } else {
                auto last = last_date(recurring);

                // If the recurring has already been triggered, we trigger again
                // for each of the missing months

                budget::date recurring_date(last.year(), last.month(), 1);

                while (!(recurring_date.year() == now.year() && recurring_date.month() == now.month())) {
                    // Get to the next month
                    recurring_date += budget::months(1);

                    generate_recurring(recurring_date, recurring);

                    changed = true;
                }
            }
        } else if (recurring.recurs == "weekly") {
            if (recurring_not_triggered(recurring)) {
                // If the recurring has never been created, we create it for
                // the first at the time of today

                if (now.week() == 53) {
                    // We do not create recurring expenses in week 52 (53-1)
                    generate_recurring((now - days(7)).start_of_week(), recurring);
                } else {
                    generate_recurring(now.start_of_week(), recurring);
                }

                changed = true;
            } else {
                auto last = last_date(recurring);

                // Note: The start_of_week() is only necessary because the user
                // could have created a matching expense in an arbitrary date
                auto recurring_date = last.start_of_week() + budget::days(7);

                while (recurring_date < now) {
                    // We skip the last week of the year since it's incomplete
                    if (recurring_date.week() < 53) {
                        generate_recurring(recurring_date, recurring);

                        changed = true;
                    }

                    // Advance by one week
                    recurring_date += budget::days(7);
                }
            }
        } else {
            cpp_unreachable("Invalid recurrence");
        }
    }

    if (changed) {
        save_expenses();
    }

    internal_config_remove("recurring:last_checked");
}

void budget::recurring_module::preload() {
    // In server mode, there is no need to generate recurring expenses
    // the server will take charge of that
    if (is_server_mode()) {
        return;
    }

    load_recurrings();
    load_accounts();
    load_expenses();

    check_for_recurrings();
}

void budget::recurring_module::load() {
    // Only need to load in server mode
    if (is_server_mode()) {
        load_recurrings();
        load_accounts();
        load_expenses();
    }
}

void budget::recurring_module::unload() {
    save_recurrings();
}

void budget::recurring_module::handle(const std::vector<std::string>& args) {
    budget::console_writer w(std::cout);

    if (args.size() == 1) {
        show_recurrings(w);
    } else {
        auto& subcommand = args[1];

        if (subcommand == "show") {
            show_recurrings(w);
        } else if (subcommand == "add") {
            recurring recurring;
            recurring.guid   = generate_guid();

            edit_string_complete(recurring.account, "Account", all_account_names(), not_empty_checker(), account_checker());
            edit_string(recurring.name, "Name", not_empty_checker());
            edit_money(recurring.amount, "Amount", not_negative_checker());
            edit_string_complete(recurring.recurs, "Recurrence", {"monthly", "weekly"}, not_empty_checker(), one_of_checker({"monthly", "weekly"}));
            edit_string_complete(recurring.type, "Type", {"expense", "earning"}, not_empty_checker(), one_of_checker({"expense", "earning"}));

            auto id = add_recurring(std::move(recurring));
            std::cout << "Recurring " << id << " has been created" << std::endl;
        } else if (subcommand == "delete") {
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if (recurrings.remove(id)) {
                std::cout << "Recurring operation " << id << " has been deleted" << std::endl;
                std::cout << "Note: The generated operations have not been deleted" << std::endl;
            } else {
                throw budget_exception("There are no recurring expense with id " + args[2]);
            }
        } else if (subcommand == "edit") {
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            auto recurring          = recurrings[id];
            auto previous_recurring = recurring; // Temporary Copy

            edit_string_complete(recurring.account, "Account", all_account_names(), not_empty_checker(), account_checker());
            edit_string(recurring.name, "Name", not_empty_checker());
            edit_money(recurring.amount, "Amount", not_negative_checker());
            edit_string_complete(recurring.recurs, "Recurrence", {"monthly","weekly"}, not_empty_checker(), one_of_checker({"monthly","weekly"}));

            if (edit_recurring(recurring, previous_recurring)) {
                std::cout << "Recurring operation " << id << " has been modified" << std::endl;
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_recurrings() {
    recurrings.load();
}

void budget::save_recurrings() {
    recurrings.save();
}

void budget::recurring::save(data_writer & writer) {
    writer << id;
    writer << guid;
    writer << account;
    writer << name;
    writer << amount;
    writer << recurs;
    writer << type;
}

void budget::recurring::load(data_reader & reader) {
    reader >> id;
    reader >> guid;
    reader >> account;
    reader >> name;
    reader >> amount;
    reader >> recurs;

    // Recurring earning support was added without a bump in database
    // so we need this check to handle older database
    if (reader.more()) {
        reader >> type;
    } else {
        type = "expense";
    }

    if (config_contains("random")) {
        amount = budget::random_money(100, 1000);
    }
}

std::vector<recurring> budget::all_recurrings() {
    return recurrings.data();
}

void budget::set_recurrings_changed() {
    recurrings.set_changed();
}

void budget::set_recurrings_next_id(size_t next_id) {
    recurrings.next_id = next_id;
}

void budget::show_recurrings(budget::writer& w) {
    w << title_begin << "Recurring operations " << add_button("recurrings") << title_end;

    if (recurrings.empty()) {
        w << "No recurring operations" << end_of_line;
    } else {
        std::vector<std::string> columns = {"ID", "Account", "Name", "Amount", "Recurs", "Type", "Edit"};
        std::vector<std::vector<std::string>> contents;

        money total;

        for (auto& recurring : recurrings.data()) {
            contents.push_back({to_string(recurring.id),
                                recurring.account,
                                recurring.name,
                                to_string(recurring.amount),
                                recurring.recurs,
                                recurring.type,
                                "::edit::recurrings::" + to_string(recurring.id)});

            total += recurring.amount;
        }

        contents.push_back({"", "", "", "", "", "", ""});
        contents.push_back({"", "", "Total", to_string(total), "", "", ""});

        w.display_table(columns, contents, 1, {}, 0, 2);
    }
}

bool budget::recurring_exists(size_t id) {
    return recurrings.exists(id);
}

void budget::recurring_delete(size_t id) {
    if (!recurrings.exists(id)) {
        throw budget_exception("There are no recurring with id ");
    }

    recurrings.remove(id);
}

recurring budget::recurring_get(size_t id) {
    return recurrings[id];
}

size_t budget::add_recurring(budget::recurring&& recurring) {
    // Create the equivalent expense

    generate_recurring(budget::local_day(), recurring);

    save_expenses();
    save_earnings();

    return recurrings.add(std::forward<budget::recurring>(recurring));
}

bool budget::edit_recurring(const budget::recurring& recurring, const budget::recurring & previous_recurring) {
    // Update the corresponding expense

    auto now = budget::local_day();

    if (recurring.type == "expense") {
        for (auto& expense : all_expenses()) {
            if (expense.date.year() == now.year() && expense.date.month() == now.month() && expense.name == previous_recurring.name
                && expense.amount == previous_recurring.amount && get_account(expense.account).name == previous_recurring.account) {
                expense.name    = recurring.name;
                expense.amount  = recurring.amount;
                expense.account = get_account(recurring.account, now.year(), now.month()).id;

                edit_expense(expense);

                break;
            }
        }

        save_expenses();
    } else if (recurring.type == "earning") {
        for (auto& earning : all_earnings()) {
            if (earning.date.year() == now.year() && earning.date.month() == now.month() && earning.name == previous_recurring.name
                && earning.amount == previous_recurring.amount && get_account(earning.account).name == previous_recurring.account) {
                earning.name    = recurring.name;
                earning.amount  = recurring.amount;
                earning.account = get_account(recurring.account, now.year(), now.month()).id;

                edit_earning(earning);

                break;
            }
        }

        save_earnings();
    } else {
        throw budget_exception("Invalid recurring type " + recurring.type);
    }

    return recurrings.indirect_edit(recurring);
}
