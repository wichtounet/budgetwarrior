//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

void accounts_page(const httplib::Request& req, httplib::Response& res);
void all_accounts_page(const httplib::Request& req, httplib::Response& res);
void add_accounts_page(const httplib::Request& req, httplib::Response& res);
void edit_accounts_page(const httplib::Request& req, httplib::Response& res);
void archive_accounts_month_page(const httplib::Request& req, httplib::Response& res);
void archive_accounts_year_page(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
