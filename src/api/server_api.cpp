//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/earnings_api.hpp"
#include "api/expenses_api.hpp"
#include "api/accounts_api.hpp"
#include "api/incomes_api.hpp"
#include "api/objectives_api.hpp"
#include "api/recurrings_api.hpp"
#include "api/debts_api.hpp"
#include "api/wishes_api.hpp"
#include "api/fortunes_api.hpp"
#include "api/assets_api.hpp"

#include "config.hpp"
#include "version.hpp"
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

} //end of anonymous namespace

void budget::load_api(httplib::Server& server) {
    server.Get("/api/server/up/", &server_up_api);
    server.Get("/api/server/version/", &server_version_api);
    server.Post("/api/server/version/support/", &server_version_support_api);

    server.Post("/api/accounts/add/", &add_accounts_api);
    server.Post("/api/accounts/edit/", &edit_accounts_api);
    server.Get("/api/accounts/delete/", &delete_accounts_api);
    server.Post("/api/accounts/archive/month/", &archive_accounts_month_api);
    server.Post("/api/accounts/archive/year/", &archive_accounts_year_api);
    server.Get("/api/accounts/list/", &list_accounts_api);

    server.Post("/api/incomes/add/", &add_incomes_api);
    server.Post("/api/incomes/edit/", &edit_incomes_api);
    server.Get("/api/incomes/delete/", &delete_incomes_api);
    server.Get("/api/incomes/list/", &list_incomes_api);

    server.Post("/api/expenses/add/", &add_expenses_api);
    server.Post("/api/expenses/edit/", &edit_expenses_api);
    server.Get("/api/expenses/delete/", &delete_expenses_api);
    server.Get("/api/expenses/list/", &list_expenses_api);

    server.Post("/api/earnings/add/", &add_earnings_api);
    server.Post("/api/earnings/edit/", &edit_earnings_api);
    server.Get("/api/earnings/delete/", &delete_earnings_api);
    server.Get("/api/earnings/list/", &list_earnings_api);

    server.Post("/api/recurrings/add/", &add_recurrings_api);
    server.Post("/api/recurrings/edit/", &edit_recurrings_api);
    server.Get("/api/recurrings/delete/", &delete_recurrings_api);
    server.Get("/api/recurrings/list/", &list_recurrings_api);

    server.Post("/api/debts/add/", &add_debts_api);
    server.Post("/api/debts/edit/", &edit_debts_api);
    server.Get("/api/debts/delete/", &delete_debts_api);
    server.Get("/api/debts/list/", &list_debts_api);

    server.Post("/api/fortunes/add/", &add_fortunes_api);
    server.Post("/api/fortunes/edit/", &edit_fortunes_api);
    server.Get("/api/fortunes/delete/", &delete_fortunes_api);
    server.Get("/api/fortunes/list/", &list_fortunes_api);

    server.Post("/api/wishes/add/", &add_wishes_api);
    server.Post("/api/wishes/edit/", &edit_wishes_api);
    server.Get("/api/wishes/delete/", &delete_wishes_api);
    server.Get("/api/wishes/list/", &list_wishes_api);

    server.Post("/api/assets/add/", &add_assets_api);
    server.Post("/api/assets/edit/", &edit_assets_api);
    server.Get("/api/assets/delete/", &delete_assets_api);
    server.Get("/api/assets/list/", &list_assets_api);

    server.Post("/api/asset_values/add/", &add_asset_values_api);
    server.Post("/api/asset_values/edit/", &edit_asset_values_api);
    server.Post("/api/asset_values/batch/", &batch_asset_values_api);
    server.Get("/api/asset_values/delete/", &delete_asset_values_api);
    server.Get("/api/asset_values/list/", &list_asset_values_api);

    server.Post("/api/asset_shares/add/", &add_asset_shares_api);
    server.Post("/api/asset_shares/edit/", &edit_asset_shares_api);
    server.Get("/api/asset_shares/delete/", &delete_asset_shares_api);
    server.Get("/api/asset_shares/list/", &list_asset_shares_api);

    server.Post("/api/asset_classes/add/", &add_asset_classes_api);
    server.Post("/api/asset_classes/edit/", &edit_asset_classes_api);
    server.Get("/api/asset_classes/delete/", &delete_asset_classes_api);
    server.Get("/api/asset_classes/list/", &list_asset_classes_api);

    server.Post("/api/retirement/configure/", &retirement_configure_api);

    server.Post("/api/objectives/add/", &add_objectives_api);
    server.Post("/api/objectives/edit/", &edit_objectives_api);
    server.Get("/api/objectives/delete/", &delete_objectives_api);
    server.Get("/api/objectives/list/", &list_objectives_api);
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
