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
#include "data.hpp"

using namespace budget;

void budget::add_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_account") || !req.has_param("input_recurs")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    try {
        recurring recurring;
        recurring.guid    = budget::generate_guid();
        recurring.account = budget::get_account(budget::to_number<size_t>(req.get_param_value("input_account"))).name;
        recurring.name    = req.get_param_value("input_name");
        recurring.amount  = budget::parse_money(req.get_param_value("input_amount"));
        recurring.recurs  = req.get_param_value("input_recurs");

        if (recurring.recurs != "monthly" && recurring.recurs != "weekly") {
            api_error(req, res, "Invalid recurring frequency");
            return;
        }

        add_recurring(std::move(recurring));

        api_success(req, res, "Recurring " + to_string(recurring.id) + " has been created", to_string(recurring.id));
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::edit_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_account") || !req.has_param("input_recurs")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::recurring_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "recurring " + id + " does not exist");
        return;
    }

    try {
        recurring recurring = recurring_get(budget::to_number<size_t>(id));
        recurring.account   = budget::get_account(budget::to_number<size_t>(req.get_param_value("input_account"))).name;
        recurring.name      = req.get_param_value("input_name");
        recurring.amount    = budget::parse_money(req.get_param_value("input_amount"));
        recurring.recurs    = req.get_param_value("input_recurs");

        if (recurring.recurs != "monthly" && recurring.recurs != "weekly") {
            api_error(req, res, "Invalid recurring frequency");
            return;
        }

        edit_recurring(recurring);

        api_success(req, res, "Recurring " + to_string(recurring.id) + " has been modified");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
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

    try {
        budget::recurring_delete(budget::to_number<size_t>(id));

        api_success(req, res, "Recurring " + id + " has been deleted");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::list_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        std::stringstream ss;

        for (auto& recurring : all_recurrings()) {
            data_writer writer;
            recurring.save(writer);
            ss << writer.to_string() << std::endl;
        }

        api_success_content(req, res, ss.str());
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}
