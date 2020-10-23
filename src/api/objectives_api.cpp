//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/objectives_api.hpp"

#include "objectives.hpp"
#include "guid.hpp"
#include "http.hpp"

using namespace budget;

void budget::add_objectives_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_name", "input_type", "input_type", "input_source", "input_operator", "input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    try {
        objective objective;
        objective.guid   = budget::generate_guid();
        objective.name   = req.get_param_value("input_name");
        objective.type   = req.get_param_value("input_type");
        objective.source = req.get_param_value("input_source");
        objective.op     = req.get_param_value("input_operator");
        objective.amount = budget::parse_money(req.get_param_value("input_amount"));
        objective.date   = budget::local_day();

        add_objective(std::move(objective));

        api_success(req, res, "objective " + to_string(objective.id) + " has been created", to_string(objective.id));
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::edit_objectives_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_name", "input_type", "input_type", "input_source", "input_operator", "input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::objective_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "objective " + id + " does not exist");
        return;
    }

    try {
        objective objective = objective_get(budget::to_number<size_t>(id));
        objective.name      = req.get_param_value("input_name");
        objective.type      = req.get_param_value("input_type");
        objective.source    = req.get_param_value("input_source");
        objective.op        = req.get_param_value("input_operator");
        objective.amount    = budget::parse_money(req.get_param_value("input_amount"));

        edit_objective(objective);

        api_success(req, res, "objective " + to_string(objective.id) + " has been modified");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::delete_objectives_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::objective_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The objective " + id + " does not exit");
        return;
    }

    try {
        budget::objective_delete(budget::to_number<size_t>(id));

        api_success(req, res, "objective " + id + " has been deleted");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::list_objectives_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        std::stringstream ss;

        for (auto& objective : all_objectives()) {
            ss << objective;
            ss << std::endl;
        }

        api_success_content(req, res, ss.str());
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}
