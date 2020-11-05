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
#include "data.hpp"

using namespace budget;

void budget::add_debts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_title") || !req.has_param("input_direction")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    try {
        debt debt;
        debt.state         = 0;
        debt.guid          = budget::generate_guid();
        debt.creation_date = budget::local_day();
        debt.direction     = req.get_param_value("input_direction") == "to";
        debt.name          = req.get_param_value("input_name");
        debt.title         = req.get_param_value("input_title");
        debt.amount        = budget::money_from_string(req.get_param_value("input_amount"));

        add_debt(std::move(debt));

        api_success(req, res, "Debt " + to_string(debt.id) + " has been created", to_string(debt.id));
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::edit_debts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_title") ||
        !req.has_param("input_direction") || !req.has_param("input_paid")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::debt_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Debt " + id + " does not exist");
        return;
    }

    try {
        debt debt      = debt_get(budget::to_number<size_t>(id));
        debt.direction = req.get_param_value("input_direction") == "to";
        debt.name      = req.get_param_value("input_name");
        debt.title     = req.get_param_value("input_title");
        debt.amount    = budget::money_from_string(req.get_param_value("input_amount"));
        debt.state     = req.get_param_value("input_paid") == "yes" ? 1 : 0;

        edit_debt(debt);

        api_success(req, res, "Debt " + to_string(debt.id) + " has been modified");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
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

    try {
        budget::debt_delete(budget::to_number<size_t>(id));

        api_success(req, res, "Debt " + id + " has been deleted");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::list_debts_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        std::stringstream ss;

        for (auto& debt : all_debts()) {
            data_writer writer;
            debt.save(writer);
            ss << writer.to_string() << std::endl;
        }

        api_success_content(req, res, ss.str());
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}
