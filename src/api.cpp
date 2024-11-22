//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "cpp_utils/assert.hpp"

#include "api.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "http.hpp"
#include "logging.hpp"

namespace {

template<typename Cli>
budget::api_response base_api_get(Cli& cli, const std::string& api) {
    auto server      = budget::config_value("server_url");
    auto server_port = budget::get_server_port();

    const std::string api_complete = "/api" + api;

    httplib::Request req;
    req.method = "GET";
    req.path = api_complete;

    req.set_header("Accept", "*/*");
    req.set_header("User-Agent", "cpp-httplib/0.1");

    if (budget::is_secure()) {
        auto user = budget::get_web_user();
        auto password = budget::get_web_password();

        std::string const authorization = "Basic " + budget::base64_encode(user + ":" + password);
        req.set_header("Authorization", authorization.c_str());
    }

    auto base_res = std::make_shared<httplib::Response>();

    std::shared_ptr<httplib::Response> res;
    auto error = httplib::Error::Success;
    if (cli.send(req, *base_res, error)) {
        res = base_res;
    } else {
        res = nullptr;
    }

    if (!res) {
        LOG_F(ERROR, "Request from the API failed: No response from server");
        LOG_F(ERROR, "API: {}:{}/{}", server, server_port, api_complete);

        return {false, ""};
    }
    if (res->status != 200) {
        LOG_F(ERROR, "Request from the API failed");
        LOG_F(ERROR, "API: {}:{}/{}", server, server_port, api_complete);
        LOG_F(ERROR, "Status: {}", res->status);
        LOG_F(ERROR, "Content: {}", res->body);

        return {false, ""};
    }
    return {true, res->body};
}

template<typename Cli>
budget::api_response base_api_post(Cli& cli, const std::string& api, const std::map<std::string, std::string, std::less<>>& params) {
    auto server      = budget::config_value("server_url");
    auto server_port = budget::get_server_port();

    const std::string api_complete = "/api" + api;

    std::string query;
    for (const auto& [key, value] : params) {
        if (!query.empty()) {
            query += "&";
        }
        query += key;
        query += "=";
        query += value;
    }

    // Add some form of identification
    if (!params.empty()) {
        query += "&budgetwarrior=666";
    }

    httplib::Request req;
    req.method = "POST";
    req.path = api_complete;

    req.set_header("Host", std::format("{}:{}", server, server_port).c_str());
    req.set_header("Accept", "*/*");
    req.set_header("User-Agent", "cpp-httplib/0.1");

    if (budget::is_secure()) {
        auto user = budget::get_web_user();
        auto password = budget::get_web_password();

        req.set_header("Authorization", ("Basic " + budget::base64_encode(user + ":" + password)).c_str());
    }

    req.set_header("Content-Type", "application/x-www-form-urlencoded");
    req.body = query;

    auto base_res = std::make_shared<httplib::Response>();

    std::shared_ptr<httplib::Response> res;
    auto error = httplib::Error::Success;
    if (cli.send(req, *base_res, error)) {
        res = base_res;
    } else {
        res = nullptr;
    }

    if (!res) {
        LOG_F(ERROR, "Request from the API failed: No response from server");
        LOG_F(ERROR, "API: {}:{}/{}", server, server_port, api_complete);

        return {false, ""};
    }

    if (res->status != 200) {
        LOG_F(ERROR, "Request from the API failed");
        LOG_F(ERROR, "API: {}:{}/{}", server, server_port, api_complete);
        LOG_F(ERROR, "Status: {}", res->status);
        LOG_F(ERROR, "Content: {}", res->body);

        return {false, ""};
    }

    return {true, res->body};
}

} // end of anonymous namespace

budget::api_response budget::api_get(const std::string& api) {
    cpp_assert(is_server_mode(), "api_get() should only be called in server mode");

    auto server      = config_value("server_url");
    auto server_port = budget::get_server_port();

    if (is_server_ssl()) {
        httplib::SSLClient cli(server.c_str(), int(server_port));
        return base_api_get(cli, api);
    }

    httplib::Client cli(server.c_str(), int(server_port));
    return base_api_get(cli, api);
}

budget::api_response budget::api_post(const std::string& api, const std::map<std::string, std::string, std::less<>>& params) {
    cpp_assert(is_server_mode(), "api_post() should only be called in server mode");

    auto server      = config_value("server_url");
    auto server_port = budget::get_server_port();

    if (is_server_ssl()) {
        httplib::SSLClient cli(server.c_str(), int(server_port));
        return base_api_post(cli, api, params);
    }

    httplib::Client cli(server.c_str(), int(server_port));
    return base_api_post(cli, api, params);
}
