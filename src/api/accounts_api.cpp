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
#include "data.hpp"

using namespace budget;

void budget::add_accounts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_name", "input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    try {
        account account;
        account.guid   = budget::generate_guid();
        account.name   = req.get_param_value("input_name");
        account.amount = budget::parse_money(req.get_param_value("input_amount"));
        account.since  = find_new_since();
        account.until  = budget::date(2099, 12, 31);

        add_account(std::move(account));

        api_success(req, res, "Account " + to_string(account.id) + " has been created", to_string(account.id));
    } catch (const budget_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
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

    try {
        account account = get_account(budget::to_number<size_t>(id));
        account.name    = req.get_param_value("input_name");
        account.amount  = budget::parse_money(req.get_param_value("input_amount"));

        set_accounts_changed();

        api_success(req, res, "Account " + to_string(account.id) + " has been modified");
    } catch (const budget_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
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

    try {
        budget::account_delete(budget::to_number<size_t>(id));

        api_success(req, res, "Account " + id + " has been deleted");
    } catch (const budget_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::list_accounts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        std::stringstream ss;

        for (auto& account : all_accounts()) {
            data_writer writer;
            account.save(writer);
            ss << writer.to_string() << std::endl;
        }

        api_success_content(req, res, ss.str());
    } catch (const budget_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::archive_accounts_month_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        budget::archive_accounts_impl(true);

        api_success(req, res, "Accounts have been migrated from the beginning of the month");
    } catch (const budget_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::archive_accounts_year_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        budget::archive_accounts_impl(false);

        api_success(req, res, "Accounts have been migrated from the beginning of the year");
    } catch (const budget_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception & e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}
