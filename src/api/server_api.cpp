//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/earnings_api.hpp"
#include "api/expenses_api.hpp"
#include "api/accounts_api.hpp"
#include "api/objectives_api.hpp"
#include "api/recurrings_api.hpp"
#include "api/debts_api.hpp"
#include "api/wishes_api.hpp"
#include "api/fortunes_api.hpp"

#include "accounts.hpp"
#include "assets.hpp"
#include "config.hpp"
#include "debts.hpp"
#include "expenses.hpp"
#include "fortune.hpp"
#include "guid.hpp"
#include "objectives.hpp"
#include "recurring.hpp"
#include "summary.hpp"
#include "version.hpp"
#include "wishes.hpp"
#include "writer.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void server_up_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    api_success_content(req, res, "yes");
}

void server_version_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    api_success_content(req, res, get_version_short());
}

void server_version_support_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"version"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto client_version = req.get_param_value("version");

    if (client_version == "1.1" || client_version == "1.1.0") {
        api_success_content(req, res, "yes");
    } else {
        api_success_content(req, res, "no");
    }
}

void retirement_configure_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_wrate", "input_roi"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    // Save the configuration
    internal_config_value("withdrawal_rate") = req.get_param_value("input_wrate");
    internal_config_value("expected_roi") = req.get_param_value("input_roi");

    save_config();

    api_success(req, res, "Retirement configuration was saved");
}

void add_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_name", "input_int_stocks", "input_dom_stocks", "input_bonds", "input_cash", "input_portfolio", "input_alloc"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    asset asset;
    asset.guid            = budget::generate_guid();
    asset.name            = req.get_param_value("input_name");
    asset.int_stocks      = budget::parse_money(req.get_param_value("input_int_stocks"));
    asset.dom_stocks      = budget::parse_money(req.get_param_value("input_dom_stocks"));
    asset.bonds           = budget::parse_money(req.get_param_value("input_bonds"));
    asset.cash            = budget::parse_money(req.get_param_value("input_cash"));
    asset.portfolio       = req.get_param_value("input_portfolio") == "yes";
    asset.portfolio_alloc = budget::parse_money(req.get_param_value("input_alloc"));
    asset.currency        = req.get_param_value("input_currency");

    if (asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != money(100)) {
        api_error(req, res, "The total allocation of the asset is not 100%");
        return;
    }

    add_asset(std::move(asset));

    api_success(req, res, "asset " + to_string(asset.id) + " has been created", to_string(asset.id));
}

void edit_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_name", "input_int_stocks", "input_dom_stocks", "input_bonds", "input_cash", "input_portfolio", "input_alloc"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "asset " + id + " does not exist");
        return;
    }

    asset& asset          = asset_get(budget::to_number<size_t>(id));
    asset.name            = req.get_param_value("input_name");
    asset.int_stocks      = budget::parse_money(req.get_param_value("input_int_stocks"));
    asset.dom_stocks      = budget::parse_money(req.get_param_value("input_dom_stocks"));
    asset.bonds           = budget::parse_money(req.get_param_value("input_bonds"));
    asset.cash            = budget::parse_money(req.get_param_value("input_cash"));
    asset.portfolio       = req.get_param_value("input_portfolio") == "yes";
    asset.portfolio_alloc = budget::parse_money(req.get_param_value("input_alloc"));
    asset.currency        = req.get_param_value("input_currency");

    if (asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != money(100)) {
        api_error(req, res, "The total allocation of the asset is not 100%");
        return;
    }

    set_assets_changed();

    api_success(req, res, "asset " + to_string(asset.id) + " has been modified");
}

void delete_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The asset " + id + " does not exit");
        return;
    }

    budget::asset_delete(budget::to_number<size_t>(id));

    api_success(req, res, "asset " + id + " has been deleted");
}

void list_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    std::stringstream ss;

    for (auto& asset : all_assets()) {
        ss << asset;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void add_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_asset", "input_date", "input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    asset_value asset_value;
    asset_value.guid     = budget::generate_guid();
    asset_value.amount   = budget::parse_money(req.get_param_value("input_amount"));
    asset_value.asset_id = budget::to_number<size_t>(req.get_param_value("input_asset"));
    asset_value.set_date = budget::from_string(req.get_param_value("input_date"));

    add_asset_value(std::move(asset_value));

    api_success(req, res, "Asset value " + to_string(asset_value.id) + " has been created", to_string(asset_value.id));
}

void edit_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_asset", "input_date", "input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_value_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Asset value " + id + " does not exist");
        return;
    }

    asset_value& asset_value = asset_value_get(budget::to_number<size_t>(id));
    asset_value.amount       = budget::parse_money(req.get_param_value("input_amount"));
    asset_value.asset_id     = budget::to_number<size_t>(req.get_param_value("input_asset"));
    asset_value.set_date     = budget::from_string(req.get_param_value("input_date"));

    set_asset_values_changed();

    api_success(req, res, "Asset " + to_string(asset_value.id) + " has been modified");
}

void delete_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_value_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The asset value " + id + " does not exit");
        return;
    }

    budget::asset_value_delete(budget::to_number<size_t>(id));

    api_success(req, res, "The asset value " + id + " has been deleted");
}

void list_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    std::stringstream ss;

    for (auto& asset_value : all_asset_values()) {
        ss << asset_value;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void batch_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    auto sorted_asset_values = all_sorted_asset_values();

    for (auto& asset : all_assets()) {
        auto input_name = "input_amount_" + budget::to_string(asset.id);

        if (req.has_param(input_name.c_str())) {
            auto new_amount = budget::parse_money(req.get_param_value(input_name.c_str()));

            budget::money current_amount;

            for (auto& asset_value : sorted_asset_values) {
                if (asset_value.asset_id == asset.id) {
                    current_amount = asset_value.amount;
                }
            }

            // If the amount changed, update it
            if (current_amount != new_amount) {
                asset_value asset_value;
                asset_value.guid     = budget::generate_guid();
                asset_value.amount   = new_amount;
                asset_value.asset_id = asset.id;
                asset_value.set_date = budget::from_string(req.get_param_value("input_date"));

                add_asset_value(std::move(asset_value));
            }
        }
    }

    api_success(req, res, "Asset values have been updated");
}

} //end of anonymous namespace

void budget::load_api(httplib::Server& server) {
    server.get("/api/server/up/", &server_up_api);
    server.get("/api/server/version/", &server_version_api);
    server.post("/api/server/version/support/", &server_version_support_api);

    server.post("/api/accounts/add/", &add_accounts_api);
    server.post("/api/accounts/edit/", &edit_accounts_api);
    server.post("/api/accounts/delete/", &delete_accounts_api);
    server.post("/api/accounts/archive/month/", &archive_accounts_month_api);
    server.post("/api/accounts/archive/year/", &archive_accounts_year_api);
    server.get("/api/accounts/list/", &list_accounts_api);

    server.post("/api/expenses/add/", &add_expenses_api);
    server.post("/api/expenses/edit/", &edit_expenses_api);
    server.post("/api/expenses/delete/", &delete_expenses_api);
    server.get("/api/expenses/list/", &list_expenses_api);

    server.post("/api/earnings/add/", &add_earnings_api);
    server.post("/api/earnings/edit/", &edit_earnings_api);
    server.post("/api/earnings/delete/", &delete_earnings_api);
    server.get("/api/earnings/list/", &list_earnings_api);

    server.post("/api/recurrings/add/", &add_recurrings_api);
    server.post("/api/recurrings/edit/", &edit_recurrings_api);
    server.post("/api/recurrings/delete/", &delete_recurrings_api);
    server.get("/api/recurrings/list/", &list_recurrings_api);

    server.post("/api/debts/add/", &add_debts_api);
    server.post("/api/debts/edit/", &edit_debts_api);
    server.post("/api/debts/delete/", &delete_debts_api);
    server.get("/api/debts/list/", &list_debts_api);

    server.post("/api/fortunes/add/", &add_fortunes_api);
    server.post("/api/fortunes/edit/", &edit_fortunes_api);
    server.post("/api/fortunes/delete/", &delete_fortunes_api);
    server.get("/api/fortunes/list/", &list_fortunes_api);

    server.post("/api/wishes/add/", &add_wishes_api);
    server.post("/api/wishes/edit/", &edit_wishes_api);
    server.post("/api/wishes/delete/", &delete_wishes_api);
    server.get("/api/wishes/list/", &list_wishes_api);

    server.post("/api/assets/add/", &add_assets_api);
    server.post("/api/assets/edit/", &edit_assets_api);
    server.post("/api/assets/delete/", &delete_assets_api);
    server.get("/api/assets/list/", &list_assets_api);

    server.post("/api/asset_values/add/", &add_asset_values_api);
    server.post("/api/asset_values/edit/", &edit_asset_values_api);
    server.post("/api/asset_values/batch/", &batch_asset_values_api);
    server.post("/api/asset_values/delete/", &delete_asset_values_api);
    server.get("/api/asset_values/list/", &list_asset_values_api);

    server.post("/api/retirement/configure/", &retirement_configure_api);

    server.post("/api/objectives/add/", &add_objectives_api);
    server.post("/api/objectives/edit/", &edit_objectives_api);
    server.post("/api/objectives/delete/", &delete_objectives_api);
    server.get("/api/objectives/list/", &list_objectives_api);
}

bool budget::api_start(const httplib::Request& req, httplib::Response& res) {
    if (is_secure()) {
        if (req.has_header("Authorization")) {
            auto authorization = req.get_header_value("Authorization");

            if (authorization.substr(0, 6) != "Basic ") {
                res.status = 401;
                res.set_header("WWW-Authenticate", "Basic realm=\"budgetwarrior\"");

                return false;
            }

            auto sub_authorization = authorization.substr(6, authorization.size());
            auto decoded           = base64_decode(sub_authorization);

            if (decoded.find(':') == std::string::npos) {
                res.status = 401;
                res.set_header("WWW-Authenticate", "Basic realm=\"budgetwarrior\"");

                return false;
            }

            auto username = decoded.substr(0, decoded.find(':'));
            auto password = decoded.substr(decoded.find(':') + 1, decoded.size());

            if (username != get_web_user()) {
                res.status = 401;
                res.set_header("WWW-Authenticate", "Basic realm=\"budgetwarrior\"");

                return false;
            }

            if (password != get_web_password()) {
                res.status = 401;
                res.set_header("WWW-Authenticate", "Basic realm=\"budgetwarrior\"");

                return false;
            }
        } else {
            res.status = 401;
            res.set_header("WWW-Authenticate", "Basic realm=\"budgetwarrior\"");

            return false;
        }
    }

    return true;
}

void budget::api_error(const httplib::Request& req, httplib::Response& res, const std::string& message) {
    if (req.has_param("server")) {
        auto url = req.get_param_value("back_page") + "?error=true&message=" + httplib::detail::encode_url(message);
        res.set_redirect(url.c_str());
    } else {
        res.set_content("Error: " + message, "text/plain");
    }
}

void budget::api_success(const httplib::Request& req, httplib::Response& res, const std::string& message) {
    if (req.has_param("server")) {
        auto url = req.get_param_value("back_page") + "?success=true&message=" + httplib::detail::encode_url(message);
        res.set_redirect(url.c_str());
    } else {
        res.set_content("Success: " + message, "text/plain");
    }
}

void budget::api_success(const httplib::Request& req, httplib::Response& res, const std::string& message, const std::string& content) {
    if (req.has_param("server")) {
        auto url = req.get_param_value("back_page") + "?success=true&message=" + httplib::detail::encode_url(message);
        res.set_redirect(url.c_str());
    } else {
        res.set_content(content, "text/plain");
    }
}

void budget::api_success_content(const httplib::Request& /*req*/, httplib::Response& res, const std::string& content) {
    res.set_content(content, "text/plain");
}

bool budget::parameters_present(const httplib::Request& req, std::vector<const char*> parameters) {
    for (auto& param : parameters) {
        if (!req.has_param(param)) {
            return false;
        }
    }

    return true;
}
