//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "expenses.hpp"
#include "args.hpp"
#include "accounts.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "writer.hpp"
#include "budget_exception.hpp"

using namespace budget;

namespace {

static data_handler<expense> expenses { "expenses", "expenses.data" };

void show_templates(){
    std::vector<std::string> columns = {"ID", "Account", "Name", "Amount"};
    std::vector<std::vector<std::string>> contents;

    size_t count = 0;

    for(auto& expense : expenses.data()){
        if(expense.date == TEMPLATE_DATE){
            contents.push_back({to_string(expense.id), get_account(expense.account).name, expense.name, to_string(expense.amount)});
            ++count;
        }
    }

    if(count == 0){
        std::cout << "No templates" << std::endl;
    } else {
        console_writer w(std::cout);
        w.display_table(columns, contents);
    }
}

} //end of anonymous namespace

std::map<std::string, std::string> budget::expense::get_params() const {
    std::map<std::string, std::string> params;

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
        show_expenses(w);
    } else {
        auto& subcommand = args[1];

        if(subcommand == "show"){
            if(args.size() == 2){
                show_expenses(w);
            } else if(args.size() == 3){
                show_expenses(budget::month(to_number<unsigned short>(args[2])), w);
            } else if(args.size() == 4){
                show_expenses(
                    budget::month(to_number<unsigned short>(args[2])),
                    budget::year(to_number<unsigned short>(args[3])), w);
            } else {
                throw budget_exception("Too many arguments to expense show");
            }
        } else if(subcommand == "all"){
            show_all_expenses(w);
        } else if(subcommand == "template"){
            show_templates();
        } else if(subcommand == "add"){
            if(args.size() > 2){
                std::string template_name;
                std::string space;

                for(size_t i = 2; i < args.size(); ++i){
                    template_name += space;
                    template_name += args[i];
                    space = " ";
                }

                bool template_found = false;

                for(auto& template_expense : all_expenses()){
                    if(template_expense.date == TEMPLATE_DATE && template_expense.name == template_name){
                        expense expense;
                        expense.guid = generate_guid();
                        expense.date = budget::local_day();
                        expense.name = template_expense.name;
                        expense.amount = template_expense.amount;
                        expense.account = template_expense.account;

                        auto id = expenses.add(std::move(expense));
                        std::cout << "Expense " << id << " has been created" << std::endl;

                        template_found = true;
                        break;
                    }
                }

                if(!template_found){
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
            } else {
                expense expense;
                expense.guid = generate_guid();
                expense.date = budget::local_day();

                std::string account_name;

                if (config_contains("default_account")) {
                    auto default_account = config_value("default_account");

                    if (account_exists(default_account)) {
                        account_name = default_account;
                    } else {
                        std::cerr << "error: The default account set in the configuration does not exist, ignoring it" << std::endl;
                    }
                }

                edit_date(expense.date, "Date");

                edit_string_complete(account_name, "Account", all_account_names(), not_empty_checker(), account_checker(expense.date));
                expense.account = get_account(account_name, expense.date.year(), expense.date.month()).id;

                edit_string(expense.name, "Name", not_empty_checker());
                edit_money(expense.amount, "Amount", not_negative_checker(), not_zero_checker());

                auto id = expenses.add(std::move(expense));
                std::cout << "Expense " << id << " has been created" << std::endl;
            }
        } else if(subcommand == "delete"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if(!expenses.exists(id)){
                throw budget_exception("There are no expense with id ");
            }

            expenses.remove(id);

            std::cout << "Expense " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            enough_args(args, 3);

            size_t id = to_number<size_t>(args[2]);

            if(!expenses.exists(id)){
                throw budget_exception("There are no expense with id " + args[2]);
            }

            auto& expense = expenses[id];

            edit_date(expense.date, "Date");

            auto account_name = get_account(expense.account).name;
            edit_string_complete(account_name, "Account", all_account_names(), not_empty_checker(), account_checker(expense.date));
            expense.account = get_account(account_name, expense.date.year(), expense.date.month()).id;

            edit_string(expense.name, "Name", not_empty_checker());
            edit_money(expense.amount, "Amount", not_negative_checker(), not_zero_checker());

            if (expenses.edit(expense)) {
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
}

void budget::load_expenses(){
    expenses.load();
}

void budget::save_expenses(){
    expenses.save();
}

void budget::add_expense(budget::expense&& expense){
    expenses.add(std::forward<budget::expense>(expense));
}

bool budget::edit_expense(expense& expense){
    return expenses.edit(expense);
}

std::ostream& budget::operator<<(std::ostream& stream, const expense& expense){
    return stream << expense.id  << ':' << expense.guid << ':' << expense.account << ':' << expense.name << ':' << expense.amount << ':' << to_string(expense.date);
}

void budget::operator>>(const std::vector<std::string>& parts, expense& expense){
    bool random = config_contains("random");

    expense.id = to_number<size_t>(parts[0]);
    expense.guid = parts[1];
    expense.account = to_number<size_t>(parts[2]);
    expense.name = parts[3];
    expense.date = from_string(parts[5]);

    if(random){
        expense.amount = budget::random_money(10, 1500);
    } else {
        expense.amount = parse_money(parts[4]);
    }
}

std::vector<expense>& budget::all_expenses(){
    return expenses.data();
}

void budget::set_expenses_changed(){
    expenses.set_changed();
}

void budget::set_expenses_next_id(size_t next_id){
    expenses.next_id = next_id;
}

void budget::show_all_expenses(budget::writer& w){
    w << title_begin << "All Expenses " << add_button("expenses") << title_end;

    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount", "Edit"};
    std::vector<std::vector<std::string>> contents;

    for(auto& expense : expenses.data()){
        contents.push_back({to_string(expense.id), to_string(expense.date), get_account(expense.account).name,
            expense.name, to_string(expense.amount), "::edit::expenses::" + to_string(expense.id)});
    }

    w.display_table(columns, contents);
}

void budget::search_expenses(const std::string& search, budget::writer& w){
    w << title_begin << "Results" << title_end;

    std::vector<std::string> columns = {"ID", "Date", "Account", "Name", "Amount", "Edit"};
    std::vector<std::vector<std::string>> contents;

    money total;
    size_t count = 0;

    auto l_search = search;
    std::transform(l_search.begin(), l_search.end(), l_search.begin(), ::tolower);

    for(auto& expense : expenses.data()){
        auto l_name = expense.name;
        std::transform(l_name.begin(), l_name.end(), l_name.begin(), ::tolower);

        if(l_name.find(l_search) != std::string::npos){
            contents.push_back({to_string(expense.id), to_string(expense.date), get_account(expense.account).name, expense.name, to_string(expense.amount), "::edit::expenses::" + to_string(expense.id)});

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

    for(auto& expense : expenses.data()){
        if(expense.date.year() == year && expense.date.month() == month){
            contents.push_back({to_string(expense.id), to_string(expense.date), get_account(expense.account).name, expense.name, to_string(expense.amount), "::edit::expenses::" + to_string(expense.id)});

            total += expense.amount;
            ++count;
        }
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

expense& budget::expense_get(size_t id) {
    if (!expenses.exists(id)) {
        throw budget_exception("There are no expense with id ");
    }

    return expenses[id];
}
