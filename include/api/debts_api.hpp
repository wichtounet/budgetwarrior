//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

namespace httplib {
struct Request;
struct Response;
};

namespace budget {

void add_debts_api(const httplib::Request& req, httplib::Response& res);
void edit_debts_api(const httplib::Request& req, httplib::Response& res);
void delete_debts_api(const httplib::Request& req, httplib::Response& res);
void list_debts_api(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
