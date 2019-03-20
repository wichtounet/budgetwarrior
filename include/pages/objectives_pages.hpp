//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

void objectives_card(budget::html_writer& w);
void list_objectives_page(const httplib::Request& req, httplib::Response& res);
void status_objectives_page(const httplib::Request& req, httplib::Response& res);
void add_objectives_page(const httplib::Request& req, httplib::Response& res);
void edit_objectives_page(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
