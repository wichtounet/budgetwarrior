//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/recurrings_api.hpp"

#include "recurring.hpp"
#include "accounts.hpp"
#include "guid.hpp"
#include "http.hpp"

using namespace budget;

void budget::add_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_account")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    recurring recurring;
    recurring.guid    = budget::generate_guid();
    recurring.account = budget::get_account(budget::to_number<size_t>(req.get_param_value("input_account"))).name;
    recurring.name    = req.get_param_value("input_name");
    recurring.amount  = budget::parse_money(req.get_param_value("input_amount"));
    recurring.recurs  = "monthly";

    add_recurring(std::move(recurring));

    api_success(req, res, "Recurring " + to_string(recurring.id) + " has been created", to_string(recurring.id));
}

void budget::edit_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_account")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::recurring_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "recurring " + id + " does not exist");
        return;
    }

    recurring recurring = recurring_get(budget::to_number<size_t>(id));
    recurring.account   = budget::get_account(budget::to_number<size_t>(req.get_param_value("input_account"))).name;
    recurring.name      = req.get_param_value("input_name");
    recurring.amount    = budget::parse_money(req.get_param_value("input_amount"));

    edit_recurring(recurring);

    api_success(req, res, "Recurring " + to_string(recurring.id) + " has been modified");
}

void budget::delete_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!req.has_param("input_id")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::recurring_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The recurring " + id + " does not exit");
        return;
    }

    budget::recurring_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Recurring " + id + " has been deleted");
}

void budget::list_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    std::stringstream ss;

    for (auto& recurring : all_recurrings()) {
        ss << recurring;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

