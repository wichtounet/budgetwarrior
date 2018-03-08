//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
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

namespace {

template<typename Cli>
budget::api_response base_api_get(Cli& cli, const std::string& api, bool silent) {
    auto server      = budget::config_value("server_url");
    auto server_port = budget::config_value("server_port");

    std::string api_complete = "/api" + api;

    httplib::Request req;
    req.method = "GET";
    req.path = api_complete.c_str();
    req.progress = [](int64_t,int64_t){};

    req.set_header("Host", (server + ":" + server_port).c_str());
    req.set_header("Accept", "*/*");
    req.set_header("User-Agent", "cpp-httplib/0.1");

    if (budget::is_secure()) {
        auto user = budget::get_web_user();
        auto password = budget::get_web_password();

        req.set_header("Authorization", ("Basic " + budget::base64_encode(user + ":" + password)).c_str());
    }

    auto base_res = std::make_shared<httplib::Response>();

    std::shared_ptr<httplib::Response> res;
    if (cli.send(req, *base_res)) {
        res = base_res;
    } else {
        res = nullptr;
    }

    if (!res) {
        if (!silent) {
            std::cerr << "Request to the API failed!" << std::endl;
            std::cerr << "  API: " << server << ":" << server_port << "/" << api_complete << std::endl;
            std::cerr << "  No response from server" << std::endl;
        }

        return {false, ""};
    } else if (res->status != 200) {
        if (!silent) {
            std::cerr << "Request to the API failed!" << std::endl;
            std::cerr << "  API: " << server << ":" << server_port << "/" << api_complete << std::endl;
            std::cerr << "  status: " << res->status << std::endl;
            std::cerr << "  content: " << res->body << std::endl;
        }

        return {false, ""};
    } else {
        return {true, res->body};
    }
}

template<typename Cli>
budget::api_response base_api_post(Cli& cli, const std::string& api, const std::map<std::string, std::string>& params) {
    auto server      = budget::config_value("server_url");
    auto server_port = budget::config_value("server_port");

    std::string api_complete = "/api" + api;

    std::string query;
    for (auto it = params.begin(); it != params.end(); ++it) {
        if (it != params.begin()) {
            query += "&";
        }
        query += it->first;
        query += "=";
        query += it->second;
    }

    // Add some form of identification
    if (!params.empty()) {
        query += "&budgetwarrior=666";
    }

    httplib::Request req;
    req.method = "POST";
    req.path = api_complete.c_str();
    req.progress = [](int64_t,int64_t){};

    req.set_header("Host", (server + ":" + server_port).c_str());
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
    if (cli.send(req, *base_res)) {
        res = base_res;
    } else {
        res = nullptr;
    }

    if (!res) {
        std::cerr << "Request to the API failed!" << std::endl;
        std::cerr << "  API: " << server << ":" << server_port << "/" << api_complete << std::endl;
        std::cerr << "  No response from server" << std::endl;

        return {false, ""};
    } else if (res->status != 200) {
        std::cerr << "Request to the API failed!" << std::endl;
        std::cerr << "  API: " << server << ":" << server_port << "/" << api_complete << std::endl;
        std::cerr << "  status: " << res->status << std::endl;
        std::cerr << "  content: " << res->body << std::endl;

        return {false, ""};
    } else {
        return {true, res->body};
    }
}

} // end of anonymous namespace

budget::api_response budget::api_get(const std::string& api, bool silent) {
    cpp_assert(is_server_mode(), "api_get() should only be called in server mode");

    auto server      = config_value("server_url");
    auto server_port = config_value("server_port");

    if(is_server_ssl()){
        httplib::SSLClient cli(server.c_str(), budget::to_number<size_t>(server_port));

        return base_api_get(cli, api, silent);
    } else {
        httplib::Client cli(server.c_str(), budget::to_number<size_t>(server_port));

        return base_api_get(cli, api, silent);
    }
}

budget::api_response budget::api_post(const std::string& api, const std::map<std::string, std::string>& params) {
    cpp_assert(is_server_mode(), "api_post() should only be called in server mode");

    auto server      = config_value("server_url");
    auto server_port = config_value("server_port");

    if(is_server_ssl()){
        httplib::SSLClient cli(server.c_str(), budget::to_number<size_t>(server_port));

        return base_api_post(cli, api, params);
    } else {
        httplib::Client cli(server.c_str(), budget::to_number<size_t>(server_port));

        return base_api_post(cli, api, params);
    }
}
