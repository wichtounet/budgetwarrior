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
#include "writer.hpp"

using namespace budget;

namespace {

static data_handler<recurring> recurrings { "recurrings", "recurrings.data" };

} //end of anonymous namespace

std::map<std::string, std::string> budget::recurring::get_params()  const {
    std::map<std::string, std::string> params;

    params["input_id"]      = budget::to_string(id);
    params["input_guid"]    = guid;
    params["input_name"]    = name;
    params["input_amount"]  = budget::to_string(amount);
    params["input_recurs"]  = recurs;
    params["input_account"] = account;

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
            auto l_year = last_year(recurring);

            if (l_year == 1400) {
                // If the recurring has never been created, we create it for
                // the first at the time of today

                budget::date recurring_date(now.year(), now.month(), 1);

                budget::expense recurring_expense;
                recurring_expense.guid    = generate_guid();
                recurring_expense.date    = recurring_date;
                recurring_expense.account = get_account(recurring.account, recurring_date.year(), recurring_date.month()).id;
                recurring_expense.amount  = recurring.amount;
                recurring_expense.name    = recurring.name;

                add_expense(std::move(recurring_expense));

                changed = true;
            } else {
                // If the recurring has already been triggered, we trigger again
                // for each of the missing months

                auto l_month = last_month(recurring, l_year);

                budget::date recurring_date(l_year, l_month, 1);

                while (!(recurring_date.year() == now.year() && recurring_date.month() == now.month())) {
                    // Get to the next month
                    recurring_date += budget::months(1);

                    budget::expense recurring_expense;
                    recurring_expense.guid    = generate_guid();
                    recurring_expense.date    = recurring_date;
                    recurring_expense.account = get_account(recurring.account, recurring_date.year(), recurring_date.month()).id;
                    recurring_expense.amount  = recurring.amount;
                    recurring_expense.name    = recurring.name;

                    add_expense(std::move(recurring_expense));

                    changed = true;
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
            recurring.recurs = "monthly";

            edit_string_complete(recurring.account, "Account", all_account_names(), not_empty_checker(), account_checker());
            edit_string(recurring.name, "Name", not_empty_checker());
            edit_money(recurring.amount, "Amount", not_negative_checker());

            // Create the equivalent expense

            auto date = budget::local_day();

            budget::expense recurring_expense;
            recurring_expense.guid    = generate_guid();
            recurring_expense.account = get_account(recurring.account, date.year(), date.month()).id;
            recurring_expense.date    = date;
            recurring_expense.amount  = recurring.amount;
            recurring_expense.name    = recurring.name;

            add_expense(std::move(recurring_expense));

            save_expenses();

            auto id = recurrings.add(std::move(recurring));
            std::cout << "Recurring expense " << id << " has been created" << std::endl;
        } else if (subcommand == "delete") {
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if (recurrings.remove(id)) {
                std::cout << "Recurring expense " << id << " has been deleted" << std::endl;
                std::cout << "Note: The generated expenses have not been deleted" << std::endl;
            } else {
                throw budget_exception("There are no recurring expense with id " + args[2]);
            }
        } else if (subcommand == "edit") {
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            auto recurring          = recurrings[id];
            auto previous_recurring = recurring; // Temporary Copy

            auto now = budget::local_day();

            edit_string_complete(recurring.account, "Account", all_account_names(), not_empty_checker(), account_checker());
            edit_string(recurring.name, "Name", not_empty_checker());
            edit_money(recurring.amount, "Amount", not_negative_checker());

            // Update the corresponding expense

            for (auto& expense : all_expenses()) {
                if (expense.date.year() == now.year() && expense.date.month() == now.month() && expense.name == previous_recurring.name && expense.amount == previous_recurring.amount && get_account(expense.account).name == previous_recurring.account) {
                    expense.name    = recurring.name;
                    expense.amount  = recurring.amount;
                    expense.account = get_account(recurring.account, now.year(), now.month()).id;

                    edit_expense(expense);

                    break;
                }
            }

            save_expenses();

            if (recurrings.indirect_edit(recurring)) {
                std::cout << "Recurring expense " << id << " has been modified" << std::endl;
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
}

void budget::recurring::load(data_reader & reader) {
    reader >> id;
    reader >> guid;
    reader >> account;
    reader >> name;
    reader >> amount;
    reader >> recurs;

    if (config_contains("random")) {
        amount = budget::random_money(100, 1000);
    }
}

budget::year budget::first_year(const budget::recurring& recurring) {
    budget::year year(1400);

    for (auto& expense : all_expenses()) {
        if (expense.name == recurring.name && expense.amount == recurring.amount && get_account(expense.account).name == recurring.account) {
            if (year == 1400 || expense.date.year() < year) {
                year = expense.date.year();
            }
        }
    }

    return year;
}

budget::month budget::first_month(const budget::recurring& recurring, budget::year year) {
    budget::month month(13);

    for (auto& expense : all_expenses()) {
        if (expense.date.year() == year && expense.name == recurring.name && expense.amount == recurring.amount && get_account(expense.account).name == recurring.account) {
            if (month == 13 || expense.date.month() < month) {
                month = expense.date.month();
            }
        }
    }

    return month;
}

budget::year budget::last_year(const budget::recurring& recurring) {
    budget::year year(1400);

    for (auto& expense : all_expenses()) {
        if (expense.name == recurring.name && expense.amount == recurring.amount && get_account(expense.account).name == recurring.account) {
            if (year == 1400 || expense.date.year() > year) {
                year = expense.date.year();
            }
        }
    }

    return year;
}

budget::month budget::last_month(const budget::recurring& recurring, budget::year year) {
    budget::month month(13);

    for (auto& expense : all_expenses()) {
        if (expense.date.year() == year && expense.name == recurring.name && expense.amount == recurring.amount && get_account(expense.account).name == recurring.account) {
            if (month == 13 || expense.date.month() > month) {
                month = expense.date.month();
            }
        }
    }

    return month;
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
    w << title_begin << "Recurring expenses " << add_button("recurrings") << title_end;

    if (recurrings.empty()) {
        w << "No recurring expenses" << end_of_line;
    } else {
        std::vector<std::string> columns = {"ID", "Account", "Name", "Amount", "Recurs", "Edit"};
        std::vector<std::vector<std::string>> contents;

        money total;

        for (auto& recurring : recurrings.data()) {
            contents.push_back({to_string(recurring.id), recurring.account, recurring.name, to_string(recurring.amount), recurring.recurs, "::edit::recurrings::" + to_string(recurring.id)});

            total += recurring.amount;
        }

        contents.push_back({"", "", "", "", "", ""});
        contents.push_back({"", "", "Total", to_string(total), "", ""});

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

void budget::add_recurring(budget::recurring&& recurring) {
    recurrings.add(std::forward<budget::recurring>(recurring));
}

void budget::edit_recurring(const budget::recurring& recurring) {
    recurrings.indirect_edit(recurring);
}
