//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/earnings_api.hpp"

#include "earnings.hpp"
#include "guid.hpp"
#include "http.hpp"

using namespace budget;

void budget::add_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_name", "input_date", "input_amount", "input_account"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    earning earning;
    earning.guid    = budget::generate_guid();
    earning.date    = budget::from_string(req.get_param_value("input_date"));
    earning.account = budget::to_number<size_t>(req.get_param_value("input_account"));
    earning.name    = req.get_param_value("input_name");
    earning.amount  = budget::parse_money(req.get_param_value("input_amount"));

    add_earning(std::move(earning));

    api_success(req, res, "Earning " + to_string(earning.id) + " has been created", to_string(earning.id));
}

void budget::edit_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_name", "input_date", "input_amount", "input_account"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::earning_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Earning " + id + " does not exist");
        return;
    }

    earning& earning = earning_get(budget::to_number<size_t>(id));
    earning.date     = budget::from_string(req.get_param_value("input_date"));
    earning.account  = budget::to_number<size_t>(req.get_param_value("input_account"));
    earning.name     = req.get_param_value("input_name");
    earning.amount   = budget::parse_money(req.get_param_value("input_amount"));

    set_earnings_changed();

    api_success(req, res, "Earning " + to_string(earning.id) + " has been modified");
}

void budget::delete_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::earning_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The earning " + id + " does not exit");
        return;
    }

    budget::earning_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Earning " + id + " has been deleted");
}

void budget::list_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    std::stringstream ss;

    for (auto& earning : all_earnings()) {
        ss << earning;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}
