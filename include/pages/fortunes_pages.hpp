//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

void list_fortunes_page(const httplib::Request& req, httplib::Response& res);
void graph_fortunes_page(const httplib::Request& req, httplib::Response& res);
void status_fortunes_page(const httplib::Request& req, httplib::Response& res);
void add_fortunes_page(const httplib::Request& req, httplib::Response& res);
void edit_fortunes_page(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
