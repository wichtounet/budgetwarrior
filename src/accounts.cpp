//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "accounts.hpp"
#include "budget_exception.hpp"
#include "args.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "earnings.hpp"
#include "expenses.hpp"
#include "writer.hpp"
#include "views.hpp"

using namespace budget;

namespace {

data_handler<account> accounts { "accounts", "accounts.data" };

size_t get_account_id(std::string_view name, budget::year year, budget::month month){
    if (auto range = accounts.data() | filter_by_name(name) | active_at_date({year, month, 5}); range) {
        return std::ranges::begin(range)->id;
    }

    return 0;
}

} //end of anonymous namespace

std::map<std::string, std::string, std::less<>> budget::account::get_params() const {
    std::map<std::string, std::string, std::less<>> params;

    params["input_id"]     = budget::to_string(id);
    params["input_guid"]   = guid;
    params["input_name"]   = name;
    params["input_amount"] = budget::to_string(amount);
    params["input_since"]  = budget::to_string(since);
    params["input_until"]  = budget::to_string(until);

    return params;
}

void budget::accounts_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::accounts_module::unload(){
    save_accounts();
    save_expenses();
    save_earnings();
}

void budget::archive_accounts_impl(bool month){
    std::vector<size_t> sources;
    std::vector<budget::account> copies;

    auto today = budget::local_day();

    budget::date until_date{};
    budget::date since_date{};

    if (month) {
        auto tmp   = budget::local_day() - months(1);
        until_date = budget::date(tmp.year(), tmp.month(), tmp.end_of_month().day());
        since_date = until_date + days(1);
    } else {
        since_date = budget::date(today.year(), 1, 1);
        until_date = since_date - days(1);
    }

    for (auto& account : accounts.data() | only_open_ended) {
        budget::account copy;
        copy.guid   = generate_guid();
        copy.name   = account.name;
        copy.amount = account.amount;
        copy.until  = budget::date(2099, 12, 31);
        copy.since  = since_date;

        account.until = until_date;

        copies.push_back(std::move(copy));

        sources.push_back(account.id);
    }

    std::unordered_map<size_t, size_t> mapping;

    for (size_t i = 0; i < copies.size(); ++i) {
        auto& copy = copies[i];

        auto id = accounts.add(std::move(copy));

        mapping[sources[i]] = id;
    }

    for (auto& expense : all_expenses() | since(since_date)) {
        if (mapping.contains(expense.account)) {
            expense.account = mapping[expense.account];
            indirect_edit_expense(expense, false);
        }
    }

    for (auto& earning : all_earnings() | since(since_date)) {
        if (mapping.contains(earning.account)) {
            earning.account = mapping[earning.account];
            indirect_edit_earning(earning, false);
        }
    }

    set_expenses_changed();
    set_earnings_changed();
    accounts.set_changed();
}

void budget::accounts_module::handle(const std::vector<std::string>& args){
    console_writer w(std::cout);

    if(args.size() == 1){
        show_accounts(w);
        return;
    }

    const auto& subcommand = args[1];

    if (subcommand == "show") {
        show_accounts(w);
    } else if (subcommand == "all") {
        show_all_accounts(w);
    } else if (subcommand == "add") {
        account account;
        account.guid  = generate_guid();
        account.since = find_new_since();
        account.until = budget::date(2099, 12, 31);

        edit_string(account.name, "Name", not_empty_checker());
        edit_money(account.amount, "Amount", not_negative_checker());

        if (account_exists(account.name)) {
            throw budget_exception("An account with this name already exists");
        }

        auto id = accounts.add(std::move(account));
        std::cout << "Account " << id << " has been created" << std::endl;
    } else if (subcommand == "delete") {
        size_t id = 0;

        if (args.size() >= 3) {
            enough_args(args, 3);

            id = to_number<size_t>(args[2]);

            if (!accounts.exists(id)) {
                throw budget_exception("There are no account with id " + args[2]);
            }
        } else {
            std::string name;
            edit_string_complete(name, "Account", all_account_names(), not_empty_checker(), account_checker());

            auto today = budget::local_day();
            id         = get_account(name, today.year(), today.month()).id;
        }

        if (all_expenses() | filter_by_account(id)) {
            throw budget_exception("There are still some expenses linked to this account, cannot delete it");
        }

        if (all_earnings() | filter_by_account(id)) {
            throw budget_exception("There are still some earnings linked to this account, cannot delete it");
        }

        accounts.remove(id);

        std::cout << "Account " << id << " has been deleted" << std::endl;
    } else if (subcommand == "edit") {
        size_t id = 0;

        auto today = budget::local_day();

        if (args.size() >= 3) {
            enough_args(args, 3);

            id = to_number<size_t>(args[2]);
        } else {
            std::string name;
            edit_string_complete(name, "Account", all_account_names(), not_empty_checker(), account_checker());

            id = get_account(name, today.year(), today.month()).id;
        }

        auto account = accounts[id];

        edit_string(account.name, "Name", not_empty_checker());

        // Verify that there are no OTHER account with this name
        // in the current set of accounts (taking archiving into account)

        if (accounts.data() | not_id(id) | active_today | filter_by_name(account.name)) {
            throw budget_exception("There is already an account with the name " + account.name);
        }

        edit_money(account.amount, "Amount", not_negative_checker());

        if (accounts.indirect_edit(account)) {
            std::cout << "Account " << id << " has been modified" << std::endl;
        }
    } else if (subcommand == "transfer") {
        std::string from_name;
        edit_string_complete(from_name, "Transfer from", all_account_names(), not_empty_checker(), account_checker());

        std::string to_name;
        edit_string_complete(to_name, "Transfer to", all_account_names(), not_empty_checker(), account_checker());

        if (from_name == to_name) {
            throw budget_exception("Cannot transfer to an from the same account");
        }

        std::string name = "Transfer";
        edit_string(name, "Transfer Name", not_empty_checker());

        money amount;
        edit_money(amount, "Amount", not_negative_checker(), not_zero_checker());

        expense expense;
        expense.guid    = generate_guid();
        expense.date    = budget::local_day();
        expense.name    = name;
        expense.amount  = amount;
        expense.account = get_account(from_name, expense.date.year(), expense.date.month()).id;

        add_expense(std::move(expense));

        earning earning;
        earning.guid    = generate_guid();
        earning.date    = budget::local_day();
        earning.name    = name;
        earning.amount  = amount;
        earning.account = get_account(to_name, earning.date.year(), earning.date.month()).id;

        add_earning(std::move(earning));
    } else if (subcommand == "migrate") {
        std::string source_account_name;
        edit_string_complete(source_account_name, "Source Account", all_account_names(), not_empty_checker(), account_checker());

        std::string destination_account_name;
        edit_string_complete(destination_account_name, "Destination Account", all_account_names(), not_empty_checker(), account_checker());

        std::cout << "This command will move all expenses and earnings from \"" << source_account_name << "\" to \"" << destination_account_name
                  << "\" and delete \"" << source_account_name << "\". Are you sure you want to proceed ? [yes/no] ? ";

        std::string answer;
        std::getline(std::cin, answer);

        if (answer == "yes" || answer == "y") {
            if (source_account_name == destination_account_name) {
                std::cout << "Migrating an account to itself has no effect" << std::endl;
            } else {
                // Make sure that we find the destination for
                // each source accounts

                for (const auto& account : accounts.data() | filter_by_name(source_account_name)) {
                    auto destination_id = get_account_id(destination_account_name, account.since.year(), account.since.month());

                    if (!destination_id) {
                        std::cout << "Impossible to find the corresponding account for account " << account.id
                                  << ". This may come from a migration issue. Open an issue on Github if you think that this is a bug" << std::endl;

                        return;
                    }
                }

                std::vector<size_t> deleted;

                // Perform the migration

                for (auto& account : accounts.data() | filter_by_name(source_account_name)) {
                    auto source_id           = account.id;
                    auto destination_account = get_account(destination_account_name, account.since.year(), account.since.month());
                    auto destination_id      = destination_account.id;

                    std::cout << "Migrate account " << source_id << " to account " << destination_id << std::endl;

                    destination_account.amount += account.amount;

                    for (auto& expense : all_expenses() | filter_by_account(source_id)) {
                        expense.account = destination_id;
                        indirect_edit_expense(expense, false);
                    }

                    for (auto& earning : all_earnings() | filter_by_account(source_id)) {
                        earning.account = destination_id;
                        indirect_edit_earning(earning, false);
                    }

                    accounts.indirect_edit(destination_account);

                    deleted.push_back(source_id);
                }

                set_expenses_changed();
                set_earnings_changed();

                // Delete the source accounts

                for (auto& id : deleted) {
                    std::cout << "Delete account " << id << std::endl;

                    accounts.remove(id);
                }

                set_accounts_changed();

                std::cout << "Migration done" << std::endl;
            }
        }
    } else if (subcommand == "archive") {
        bool month = true;

        if (args.size() == 3) {
            const auto& sub_sub_command = args[2];

            if (sub_sub_command == "month") {
                month = true;
            } else if (sub_sub_command == "year") {
                month = false;
            } else {
                throw budget_exception("Invalid subcommand \"" + subcommand + +" " + sub_sub_command + "\"");
            }
        }

        if (month) {
            std::cout
                    << "This command will create new accounts that will be used starting from the beginning of the current month. Are you sure you want to proceed ? [yes/no] ? ";
        } else {
            std::cout
                    << "This command will create new accounts that will be used starting from the beginning of the current year. Are you sure you want to proceed ? [yes/no] ? ";
        }

        std::string answer;
        std::cin >> answer;

        if (answer == "yes" || answer == "y") {
            archive_accounts_impl(month);
        }
    } else {
        throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
    }
}

void budget::load_accounts(){
    accounts.load();
}

void budget::save_accounts(){
    accounts.save();
}

budget::account budget::get_account(size_t id){
    return accounts[id];
}

budget::account budget::get_account(std::string_view name, budget::year year, budget::month month) {
    if (auto range = accounts.data() | active_at_date({year, month, 5}) | filter_by_name(name); range) {
        return *std::ranges::begin(range);
    }

    cpp_unreachable("The account does not exist");
}

void budget::account::save(data_writer& writer) const {
    writer << id;
    writer << guid;
    writer << name;
    writer << amount;
    writer << since;
    writer << until;
}

void budget::account::load(data_reader & reader){
    reader >> id;
    reader >> guid;
    reader >> name;
    reader >> amount;
    reader >> since;
    reader >> until;

    if (config_contains("random")) {
        amount = budget::random_money(1000, 10000);
    }
}

bool budget::account_exists(const std::string& name){
    return !!(accounts.data() | filter_by_name(name));
}

std::vector<account> budget::all_accounts(){
    return accounts.data();
}

std::vector<budget::account> budget::current_accounts(data_cache & cache){
    auto today = budget::local_day();
    return all_accounts(cache, today.year(), today.month());
}

std::vector<account> budget::all_accounts(data_cache & cache, budget::year year, budget::month month){
    return to_vector(cache.accounts() | active_at_date({year, month, 5}));
}

void budget::set_accounts_changed(){
    accounts.set_changed();
}

void budget::set_accounts_next_id(size_t next_id){
    accounts.next_id = next_id;
}

std::vector<std::string> budget::all_account_names(){
    return to_vector(accounts.data() | active_today | to_name);
}

void budget::show_accounts(budget::writer& w){
    w << title_begin << "Accounts " << add_button("accounts") << title_end;

    std::vector<std::string> columns = {"ID", "Name", "Amount", "Part", "Edit"};
    std::vector<std::vector<std::string>> contents;

    // Compute the total

    money total;

    for(const auto& account : w.cache.accounts() | only_open_ended){
        total += account.amount;
    }

    // Display the accounts

    for(const auto& account : w.cache.accounts() | only_open_ended){
        const float part = 100.0 * (account.amount.value / float(total.value));

        char buffer[32];
        snprintf(buffer, 32, "%.2f%%", part);

        contents.push_back({to_string(account.id), account.name,
            to_string(account.amount),
            buffer,
            "::edit::accounts::" + to_string(account.id)});
    }

    contents.push_back({"", "Total", to_string(total), "", ""});

    w.display_table(columns, contents, 1, {}, 0, 1);
}

void budget::show_all_accounts(budget::writer& w){
    w << title_begin << "All Accounts " << add_button("accounts") << title_end;

    std::vector<std::string> columns = {"ID", "Name", "Amount", "Since", "Until", "Edit"};
    std::vector<std::vector<std::string>> contents;

    for(const auto& account : w.cache.accounts()){
        contents.push_back({to_string(account.id), account.name, to_string(account.amount), to_string(account.since), to_string(account.until), "::edit::accounts::" + to_string(account.id)});
    }

    w.display_table(columns, contents);
}

bool budget::account_exists(size_t id){
    return accounts.exists(id);
}

void budget::account_delete(size_t id) {
    if (!accounts.exists(id)) {
        throw budget_exception("There are no account with id ");
    }

    accounts.remove(id);
}

bool budget::no_accounts() {
    return accounts.empty();
}

size_t budget::add_account(budget::account&& account){
    return accounts.add(std::move(account));
}

bool budget::edit_account(const budget::account& account){
    return accounts.indirect_edit(account);
}

budget::date budget::find_new_since(){
    budget::date date(1400,1,1);

    for(const auto& account : accounts.data() | not_open_ended){
        if(account.until - days(1) > date){
            date = account.until - days(1);
        }
    }

    return date;
}

bool budget::has_default_account() {
    auto account_name = user_config_value("default_account", "");

    if (account_name.empty()) {
        return false;
    }

    if (account_exists(account_name)) {
        // TODO: We should check if it exists in the current archive version
        return true;
    }

    return false;
}

account budget::default_account() {
    if (!has_default_account()) {
        throw budget_exception("No default account");
    }

    auto account_name = user_config_value("default_account", "");

    // TODO: We should check if it exists in the current archive version
    auto today = local_day();
    return get_account(account_name, today.year(), today.month());
}

bool budget::has_taxes_account() {
    auto account_name = user_config_value("taxes_account", "");

    if (account_name.empty()) {
        return false;
    }

    if (account_exists(account_name)) {
        // TODO: We should check if it exists in the current archive version
        return true;
    }

    return false;
}

account budget::taxes_account() {
    if (!has_taxes_account()) {
        throw budget_exception("No taxes account");
    }

    auto account_name = user_config_value("taxes_account", "");

    // TODO: We should check if it exists in the current archive version
    auto today = local_day();
    return get_account(account_name, today.year(), today.month());
}
