//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
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

    httplib::Client cli(server.c_str(), budget::to_number<size_t>(server_port));

    std::string api_complete = "/api" + api;

    auto res = cli.get(api_complete.c_str());

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

    httplib::Map api_params;

    for (auto& v : params) {
        api_params[v.first] = v.second;
    }

    auto server      = config_value("server_url");
    auto server_port = config_value("server_port");

    httplib::Client cli(server.c_str(), budget::to_number<size_t>(server_port));

    std::string api_complete = "/api" + api;

    auto res = cli.post(api_complete.c_str(), api_params);

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
