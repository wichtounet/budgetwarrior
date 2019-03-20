//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

void time_graph_income_page(const httplib::Request& req, httplib::Response& res);
void time_graph_earnings_page(const httplib::Request& req, httplib::Response& res);
void add_earnings_page(const httplib::Request& req, httplib::Response& res);
void edit_earnings_page(const httplib::Request& req, httplib::Response& res);
void earnings_page(const httplib::Request& req, httplib::Response& res);
void all_earnings_page(const httplib::Request& req, httplib::Response& res);
void month_breakdown_income_graph(budget::html_writer& w, const std::string& title,
                                  budget::month month, budget::year year, bool mono = false, 
                                  const std::string& style = "");

} //end of namespace budget
