//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "expenses.hpp"
#include "args.hpp"
#include "accounts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "views.hpp"
#include "writer.hpp"
#include "budget_exception.hpp"

using namespace budget;

namespace {

data_handler<expense> expenses{"expenses", "expenses.data"};

void show_templates() {
    std::vector<std::string>              columns = {"ID", "Account", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    size_t count = 0;

    for (const auto& expense : all_expenses() | template_only) {
        contents.push_back({to_string(expense.id), get_account(expense.account).name, expense.name, to_string(expense.amount)});
        ++count;
    }

    if (count == 0) {
        std::cout << "No templates" << std::endl;
    } else {
        console_writer w(std::cout);
        w.display_table(columns, contents);
    }
}

} //end of anonymous namespace

std::map<std::string, std::string, std::less<>> budget::expense::get_params() const {
    std::map<std::string, std::string, std::less<>> params;

    params["input_id"]      = budget::to_string(id);
    params["input_guid"]    = guid;
    params["input_date"]    = budget::to_string(date);
    params["input_name"]    = name;
    params["input_account"] = budget::to_string(account);
    params["input_amount"]  = budget::to_string(amount);

    return params;
}

void budget::expenses_module::load(){
    load_expenses();
    load_accounts();
}

void budget::expenses_module::unload(){
    save_expenses();
}

void budget::expenses_module::handle(const std::vector<std::string>& args){
    console_writer w(std::cout);

    if(args.size() == 1){
        return show_expenses(w);
    }

    const auto& subcommand = args[1];

    if (subcommand == "show") {
        if (args.size() == 2) {
            show_expenses(w);
        } else if (args.size() == 3) {
            show_expenses(budget::month(to_number<unsigned short>(args[2])), w);
        } else if (args.size() == 4) {
            show_expenses(budget::month(to_number<unsigned short>(args[2])), budget::year(to_number<unsigned short>(args[3])), w);
        } else {
            throw budget_exception("Too many arguments to expense show");
        }
    } else if (subcommand == "all") {
        show_all_expenses(w);
    } else if (subcommand == "template") {
        show_templates();
    } else if (subcommand == "add" && args.size() > 2) {
        std::string template_name;
        std::string space;

        for (size_t i = 2; i < args.size(); ++i) {
            template_name += space;
            template_name += args[i];
            space = " ";
        }

        if (auto range = all_expenses() | template_only | filter_by_name(template_name); range) {
            auto& template_expense = *std::ranges::begin(range);

            expense expense;
            expense.guid    = generate_guid();
            expense.date    = budget::local_day();
            expense.name    = template_expense.name;
            expense.amount  = template_expense.amount;
            expense.account = template_expense.account;

            auto id = expenses.add(std::move(expense));
            std::cout << "Expense " << id << " has been created" << std::endl;
        } else {
            std::cout << "Template \"" << template_name << "\" not found, creating a new template" << std::endl;

            expense expense;
            expense.guid = generate_guid();
            expense.date = TEMPLATE_DATE;
            expense.name = template_name;

            std::string account_name;
            edit_string_complete(account_name, "Account", all_account_names(), not_empty_checker(), account_checker());
            expense.account = get_account(account_name, expense.date.year(), expense.date.month()).id;

            edit_money(expense.amount, "Amount", not_negative_checker(), not_zero_checker());

            auto id = expenses.add(std::move(expense));
            std::cout << "Template " << id << " has been created" << std::endl;
        }
    } else if (subcommand == "add") {
        expense expense;
        expense.guid = generate_guid();
        expense.date = budget::local_day();

        std::string account_name;
        if (has_default_account()) {
            account_name = default_account().name;
        }

        edit_date(expense.date, "Date");

        edit_string_complete(account_name, "Account", all_account_names(), not_empty_checker(), account_checker(expense.date));
        expense.account = get_account(account_name, expense.date.year(), expense.date.month()).id;

        edit_string(expense.name, "Name", not_empty_checker());
        edit_money(expense.amount, "Amount", not_negative_checker(), not_zero_checker());

        auto id = expenses.add(std::move(expense));
        std::cout << "Expense " << id << " has been created" << std::endl;
    } else if (subcommand == "delete") {
        enough_args(args, 3);

        const auto id = to_number<size_t>(args[2]);

        if (expenses.remove(id)) {
            std::cout << "Expense " << id << " has been deleted" << std::endl;
        } else {
            throw budget_exception("There are no expense with id ");
        }
    } else if (subcommand == "edit") {
        enough_args(args, 3);

        const auto id = to_number<size_t>(args[2]);

        auto expense = expenses[id];

        edit_date(expense.date, "Date");

        auto account_name = get_account(expense.account).name;
        edit_string_complete(account_name, "Account", all_account_names(), not_empty_checker(), account_checker(expense.date));
        expense.account = get_account(account_name, expense.date.year(), expense.date.month()).id;

        edit_string(expense.name, "Name", not_empty_checker());
        edit_money(expense.amount, "Amount", not_negative_checker(), not_zero_checker());

        if (expenses.indirect_edit(expense)) {
            std::cout << "Expense " << id << " has been modified" << std::endl;
        }
    } else if (subcommand == "search") {
        std::string search;
        edit_string(search, "Search", not_empty_checker());

        search_expenses(search, w);
    } else {
        throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
    }
}

void budget::load_expenses(){
    expenses.load();
}

void budget::save_expenses(){
    expenses.save();
}

size_t budget::add_expense(budget::expense&& expense){
    return expenses.add(std::move(expense));
}

bool budget::edit_expense(const expense& expense){
    return expenses.indirect_edit(expense);
}

void budget::expense::save(data_writer& writer) const {
    writer << id;
    writer << guid;
    writer << account;
    writer << name;
    writer << amount;
    writer << date;
    writer << original_name;
    writer << temporary;
}

void budget::expense::load(data_reader & reader){
    reader >> id;
    reader >> guid;
    reader >> account;
    reader >> name;
    reader >> amount;
    reader >> date;
    reader >> original_name;
    reader >> temporary;

    if (config_contains("random")) {
        amount = budget::random_money(10, 1500);
    }
}

std::vector<expense> budget::all_expenses(){
    return expenses.data();
}

bool budget::indirect_edit_expense(const expense & expense, bool propagate) {
    return expenses.indirect_edit(expense, propagate);
}

void budget::set_expenses_changed(){
    expenses.set_changed();
}

void budget::show_all_expenses(budget::writer& w){
    w << title_begin << "All Expenses " << add_button("expenses") << title_end;

    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount", "Edit"};
    std::vector<std::vector<std::string>> contents;

    for (auto& expense : all_expenses()) {
        contents.push_back({to_string(expense.id),
                            to_string(expense.date),
                            get_account(expense.account).name,
                            expense.name,
                            to_string(expense.amount),
                            "::edit::expenses::" + to_string(expense.id)});
    }

    w.display_table(columns, contents);
}

void budget::search_expenses(std::string_view search, budget::writer& w){
    w << title_begin << "Results" << title_end;

    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount", "Edit"};
    std::vector<std::vector<std::string>> contents;

    money total;
    size_t count = 0;

    for (auto& expense : all_expenses()) {
        auto it = std::ranges::search(
                expense.name, search, [](char a, char b) { return std::tolower(a) == std::tolower(b); });

        if (it) {
            contents.push_back({to_string(expense.id),
                                to_string(expense.date),
                                get_account(expense.account).name,
                                expense.name,
                                to_string(expense.amount),
                                "::edit::expenses::" + to_string(expense.id)});

            total += expense.amount;
            ++count;
        }
    }

    if(count == 0){
        w << "No expenses found" << end_of_line;
    } else {
        contents.push_back({"", "", "", "Total", to_string(total), ""});

        w.display_table(columns, contents, 1, {}, 0, 1);
    }
}

void budget::show_expenses(budget::month month, budget::year year, budget::writer& w){
    w << title_begin << "Expenses of " << month << " " << year << " "
      << add_button("expenses")
      << budget::year_month_selector{"expenses", year, month} << title_end;

    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount", "Edit"};
    std::vector<std::vector<std::string>> contents;

    money total;
    size_t count = 0;

    for (auto& expense : all_expenses() | filter_by_date(year, month)) {
        contents.push_back({to_string(expense.id),
                            to_string(expense.date),
                            get_account(expense.account).name,
                            expense.name,
                            to_string(expense.amount),
                            "::edit::expenses::" + to_string(expense.id)});

        total += expense.amount;
        ++count;
    }

    if(count == 0){
        w << "No expenses for " << month << "-" << year << end_of_line;
    } else {
        contents.push_back({"", "", "", "Total", to_string(total), ""});

        w.display_table(columns, contents, 1, {}, 0, 1);
    }
}

void budget::show_expenses(budget::month month, budget::writer& w){
    auto today = budget::local_day();

    show_expenses(month, today.year(), w);
}

void budget::show_expenses(budget::writer& w){
    auto today = budget::local_day();

    show_expenses(today.month(), today.year(), w);
}

bool budget::expense_exists(size_t id){
    return expenses.exists(id);
}

void budget::expense_delete(size_t id) {
    if (!expenses.exists(id)) {
        throw budget_exception("There are no expense with id ");
    }

    expenses.remove(id);
}

expense budget::expense_get(size_t id) {
    return expenses[id];
}

void budget::migrate_expenses_9_to_10(){
    expenses.load([](data_reader & reader, expense& expense){
        reader >> expense.id;
        reader >> expense.guid;
        reader >> expense.account;
        reader >> expense.name;
        reader >> expense.amount;
        reader >> expense.date;

        // Version 10 added support for importing
        expense.original_name = "";
        expense.temporary = false;
    });

    set_expenses_changed();

    expenses.save();
}
