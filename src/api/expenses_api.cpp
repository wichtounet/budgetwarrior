//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/expenses_api.hpp"

#include "expenses.hpp"
#include "guid.hpp"
#include "http.hpp"

using namespace budget;

void budget::add_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_name", "input_date", "input_amount", "input_account"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    expense expense;
    expense.guid    = budget::generate_guid();
    expense.date    = budget::from_string(req.get_param_value("input_date"));
    expense.account = budget::to_number<size_t>(req.get_param_value("input_account"));
    expense.name    = req.get_param_value("input_name");
    expense.amount  = budget::parse_money(req.get_param_value("input_amount"));

    add_expense(std::move(expense));

    api_success(req, res, "Expense " + to_string(expense.id) + " has been created", to_string(expense.id));
}

void budget::edit_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_name", "input_date", "input_amount", "input_account"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::expense_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Expense " + id + " does not exist");
        return;
    }

    expense& expense = expense_get(budget::to_number<size_t>(id));
    expense.date     = budget::from_string(req.get_param_value("input_date"));
    expense.account  = budget::to_number<size_t>(req.get_param_value("input_account"));
    expense.name     = req.get_param_value("input_name");
    expense.amount   = budget::parse_money(req.get_param_value("input_amount"));

    set_expenses_changed();

    api_success(req, res, "Expense " + to_string(expense.id) + " has been modified");
}

void budget::delete_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::expense_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The expense " + id + " does not exit");
        return;
    }

    budget::expense_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Expense " + id + " has been deleted");
}

void budget::list_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    std::stringstream ss;

    for (auto& expense : all_expenses()) {
        ss << expense;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}
