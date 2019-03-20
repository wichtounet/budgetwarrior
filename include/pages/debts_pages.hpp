//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

// Debts Pages
void list_debts_page(const httplib::Request& req, httplib::Response& res);
void all_debts_page(const httplib::Request& req, httplib::Response& res);
void edit_debts_page(const httplib::Request& req, httplib::Response& res);
void add_debts_page(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
