//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "server_pages.hpp"

namespace budget {

void overview_page(const httplib::Request& req, httplib::Response& res);
void overview_aggregate_all_page(const httplib::Request& req, httplib::Response& res);
void overview_aggregate_year_page(const httplib::Request& req, httplib::Response& res);
void overview_aggregate_year_month_page(const httplib::Request& req, httplib::Response& res);
void overview_aggregate_month_page(const httplib::Request& req, httplib::Response& res);
void overview_year_page(const httplib::Request& req, httplib::Response& res);
void time_graph_savings_rate_page(const httplib::Request& req, httplib::Response& res);
void time_graph_tax_rate_page(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
