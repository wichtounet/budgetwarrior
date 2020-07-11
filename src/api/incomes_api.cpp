//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/incomes_api.hpp"

#include "incomes.hpp"
#include "guid.hpp"
#include "http.hpp"

using namespace budget;

void budget::add_incomes_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto amount = budget::parse_money(req.get_param_value("input_amount"));

    auto& income = budget::new_income(amount, false);

    api_success(req, res, "Income " + to_string(income.id) + " has been created", to_string(income.id));
}

void budget::edit_incomes_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::income_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Income " + id + " does not exist");
        return;
    }

    income& income = income_get(budget::to_number<size_t>(id));
    income.amount   = budget::parse_money(req.get_param_value("input_amount"));

    set_incomes_changed();

    api_success(req, res, "Income " + to_string(income.id) + " has been modified");
}

void budget::delete_incomes_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::income_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The income " + id + " does not exit");
        return;
    }

    budget::income_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Income " + id + " has been deleted");
}

void budget::list_incomes_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    std::stringstream ss;

    for (auto& income : all_incomes()) {
        ss << income;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}
