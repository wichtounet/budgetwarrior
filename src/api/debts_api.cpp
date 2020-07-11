//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/debts_api.hpp"

#include "debts.hpp"
#include "accounts.hpp"
#include "guid.hpp"
#include "http.hpp"

using namespace budget;

void budget::add_debts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_title") || !req.has_param("input_direction")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    debt debt;
    debt.state         = 0;
    debt.guid          = budget::generate_guid();
    debt.creation_date = budget::local_day();
    debt.direction     = req.get_param_value("input_direction") == "to";
    debt.name          = req.get_param_value("input_name");
    debt.title         = req.get_param_value("input_title");
    debt.amount        = budget::parse_money(req.get_param_value("input_amount"));

    add_debt(std::move(debt));

    api_success(req, res, "Debt " + to_string(debt.id) + " has been created", to_string(debt.id));
}

void budget::edit_debts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_title") || !req.has_param("input_direction") || !req.has_param("input_paid")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::debt_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Debt " + id + " does not exist");
        return;
    }

    debt& debt     = debt_get(budget::to_number<size_t>(id));
    debt.direction = req.get_param_value("input_direction") == "to";
    debt.name      = req.get_param_value("input_name");
    debt.title     = req.get_param_value("input_title");
    debt.amount    = budget::parse_money(req.get_param_value("input_amount"));
    debt.state     = req.get_param_value("input_paid") == "yes" ? 1 : 0;

    set_debts_changed();

    api_success(req, res, "Debt " + to_string(debt.id) + " has been modified");
}

void budget::delete_debts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!req.has_param("input_id")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::debt_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The debt " + id + " does not exit");
        return;
    }

    budget::debt_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Debt " + id + " has been deleted");
}

void budget::list_debts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    std::stringstream ss;

    for (auto& debt : all_debts()) {
        ss << debt;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}
