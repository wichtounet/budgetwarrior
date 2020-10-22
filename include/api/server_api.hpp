//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <string>
#include <vector>

#include "budget_exception.hpp"

namespace httplib {
struct Server;
struct Request;
struct Response;
};

namespace budget {

// For the server
void load_api(httplib::Server& server);

// For the API pages
bool api_start(const httplib::Request& req, httplib::Response& res);
void api_error(const httplib::Request& req, httplib::Response& res, const std::string& message);
void api_success(const httplib::Request& req, httplib::Response& res, const std::string& message);
void api_success(const httplib::Request& req, httplib::Response& res, const std::string& message, const std::string& content);
void api_success_content(const httplib::Request& req, httplib::Response& res, const std::string& content);
bool parameters_present(const httplib::Request& req, std::vector<const char*> parameters);

} //end of namespace budget
