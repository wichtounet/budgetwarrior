//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/accounts_api.hpp"

#include "accounts.hpp"
#include "guid.hpp"
#include "http.hpp"

using namespace budget;

void budget::add_accounts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_name", "input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    account account;
    account.guid   = budget::generate_guid();
    account.name   = req.get_param_value("input_name");
    account.amount = budget::parse_money(req.get_param_value("input_amount"));
    account.since  = find_new_since();
    account.until  = budget::date(2099, 12, 31);

    add_account(std::move(account));

    api_success(req, res, "Account " + to_string(account.id) + " has been created", to_string(account.id));
}

void budget::edit_accounts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_name", "input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::account_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "account " + id + " does not exist");
        return;
    }

    account account = account_get(budget::to_number<size_t>(id));
    account.name    = req.get_param_value("input_name");
    account.amount  = budget::parse_money(req.get_param_value("input_amount"));

    set_accounts_changed();

    api_success(req, res, "Account " + to_string(account.id) + " has been modified");
}

void budget::delete_accounts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::account_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The account " + id + " does not exit");
        return;
    }

    budget::account_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Account " + id + " has been deleted");
}

void budget::list_accounts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    std::stringstream ss;

    for (auto& account : all_accounts()) {
        ss << account;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void budget::archive_accounts_month_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    budget::archive_accounts_impl(true);

    api_success(req, res, "Accounts have been migrated from the beginning of the month");
}

void budget::archive_accounts_year_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    budget::archive_accounts_impl(false);

    api_success(req, res, "Accounts have been migrated from the beginning of the year");
}
