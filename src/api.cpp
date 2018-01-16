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

#include "httplib.h"

budget::api_response budget::api_get(const std::string& api, bool silent) {
    cpp_assert(is_server_mode(), "api_get() should only be called in server mode");

    auto server      = config_value("server_url");
    auto server_port = config_value("server_port");

    std::string api_complete = "/api" + api;

    httplib::Client cli(server.c_str(), budget::to_number<size_t>(server_port));

    httplib::Request req;
    req.method = "GET";
    req.path = api_complete.c_str();
    req.progress = [](int64_t,int64_t){};

    req.set_header("Host", (server + ":" + server_port).c_str());
    req.set_header("Accept", "*/*");
    req.set_header("User-Agent", "cpp-httplib/0.1");
    req.set_header("Authorization", ("Basic " + base64_encode(get_web_user() + ":" + get_web_password())).c_str());

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

budget::api_response budget::api_post(const std::string& api, const std::map<std::string, std::string>& params) {
    cpp_assert(is_server_mode(), "api_post() should only be called in server mode");

    auto server      = config_value("server_url");
    auto server_port = config_value("server_port");

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

    httplib::Client cli(server.c_str(), budget::to_number<size_t>(server_port));

    httplib::Request req;
    req.method = "POST";
    req.path = api_complete.c_str();
    req.progress = [](int64_t,int64_t){};

    req.set_header("Host", (server + ":" + server_port).c_str());
    req.set_header("Accept", "*/*");
    req.set_header("User-Agent", "cpp-httplib/0.1");
    req.set_header("Authorization", ("Basic " + base64_encode(get_web_user() + ":" + get_web_password())).c_str());

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
