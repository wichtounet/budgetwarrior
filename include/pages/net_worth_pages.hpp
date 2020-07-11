//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

struct asset;

// Net Worth Pages
void assets_card(budget::html_writer& w);
void net_worth_graph(budget::html_writer& w, const std::string style = "", bool card = false);
void net_worth_status_page(const httplib::Request& req, httplib::Response& res);
void net_worth_small_status_page(const httplib::Request& req, httplib::Response& res);
void net_worth_graph_page(const httplib::Request& req, httplib::Response& res);
void net_worth_allocation_page(const httplib::Request& req, httplib::Response& res);
void portfolio_allocation_page(const httplib::Request& req, httplib::Response& res);
void net_worth_currency_page(const httplib::Request& req, httplib::Response& res);
void portfolio_status_page(const httplib::Request& req, httplib::Response& res);
void portfolio_currency_page(const httplib::Request& req, httplib::Response& res);
void portfolio_graph_page(const httplib::Request& req, httplib::Response& res);
void rebalance_page(const httplib::Request& req, httplib::Response& res);
void rebalance_nocash_page(const httplib::Request& req, httplib::Response& res);

void asset_graph(budget::html_writer& w, const std::string style, asset& asset);
void asset_graph_page(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
