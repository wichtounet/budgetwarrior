//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <string>
#include <vector>

#include "date.hpp"

namespace httplib {
struct Server;
struct Response;
struct Request;
};

namespace budget {

struct writer;
struct html_writer;

void load_pages(httplib::Server& server);

bool page_start(const httplib::Request& req, httplib::Response& res, std::stringstream& content_stream, const std::string& title);
bool page_get_start(const httplib::Request& req, httplib::Response& res,
                    std::stringstream& content_stream, const std::string& title, std::vector<const char*> parameters);
void page_end(budget::html_writer& w, const httplib::Request& req, httplib::Response& res);

void display_error_message(budget::writer& w, const std::string& message);

void make_tables_sortable(budget::html_writer& w);

// Forms
void form_begin(budget::writer& w, const std::string& action, const std::string& back_page);
void page_form_begin(budget::writer& w, const std::string& action);
void form_begin_edit(budget::writer& w, const std::string& action, const std::string& back_page, const std::string& input_id);
void form_end(budget::writer& w, const std::string& button = "");

void add_text_picker(budget::writer& w, const std::string& title, const std::string& name, const std::string& default_value);
void add_name_picker(budget::writer& w, const std::string& default_value = "");
void add_title_picker(budget::writer& w, const std::string& default_value = "");
void add_amount_picker(budget::writer& w, const std::string& default_value = "");
void add_paid_amount_picker(budget::writer& w, const std::string& default_value = "");
void add_yes_no_picker(budget::writer& w, const std::string& title, const std::string& name, bool default_value);
void add_paid_picker(budget::writer& w, bool paid);
void add_date_picker(budget::writer& w, const std::string& default_value = "", bool one_line = false);
void add_account_picker(budget::writer& w, budget::date day, const std::string& default_value = "");
void add_asset_picker(budget::writer& w, const std::string& default_value = "");
void add_money_picker(budget::writer& w, const std::string& title, const std::string& name, 
                      const std::string& default_value, bool one_line = false, const std::string& currency = "");

// Charts
std::stringstream start_chart_base(budget::html_writer& w, const std::string& chart_type, 
                                   const std::string& id = "container", std::string style = "");
std::stringstream start_chart(budget::html_writer& w, const std::string& title, const std::string& chart_type,
                              const std::string& id = "container", std::string style = "");
std::stringstream start_time_chart(budget::html_writer& w, const std::string& title, const std::string& chart_type, 
                                   const std::string& id = "container", std::string style = "");
void end_chart(budget::html_writer& w, std::stringstream& ss);

// Fortunes pages
void list_fortunes_page(const httplib::Request& req, httplib::Response& res);
void graph_fortunes_page(const httplib::Request& req, httplib::Response& res);
void status_fortunes_page(const httplib::Request& req, httplib::Response& res);
void add_fortunes_page(const httplib::Request& req, httplib::Response& res);
void edit_fortunes_page(const httplib::Request& req, httplib::Response& res);

// Accounts pages
void accounts_page(const httplib::Request& req, httplib::Response& res);
void all_accounts_page(const httplib::Request& req, httplib::Response& res);
void add_accounts_page(const httplib::Request& req, httplib::Response& res);
void edit_accounts_page(const httplib::Request& req, httplib::Response& res);
void archive_accounts_month_page(const httplib::Request& req, httplib::Response& res);
void archive_accounts_year_page(const httplib::Request& req, httplib::Response& res);

// Earnings / Income pages
void time_graph_income_page(const httplib::Request& req, httplib::Response& res);
void time_graph_earnings_page(const httplib::Request& req, httplib::Response& res);
void add_earnings_page(const httplib::Request& req, httplib::Response& res);
void edit_earnings_page(const httplib::Request& req, httplib::Response& res);
void earnings_page(const httplib::Request& req, httplib::Response& res);
void all_earnings_page(const httplib::Request& req, httplib::Response& res);
void month_breakdown_income_graph(budget::html_writer& w, const std::string& title,
                                  budget::month month, budget::year year, bool mono = false, 
                                  const std::string& style = "");

// Expenses pages
void expenses_page(const httplib::Request& req, httplib::Response& res);
void search_expenses_page(const httplib::Request& req, httplib::Response& res);
void time_graph_expenses_page(const httplib::Request& req, httplib::Response& res);
void all_expenses_page(const httplib::Request& req, httplib::Response& res);
void month_breakdown_expenses_page(const httplib::Request& req, httplib::Response& res);
void year_breakdown_expenses_page(const httplib::Request& req, httplib::Response& res);
void add_expenses_page(const httplib::Request& req, httplib::Response& res);
void edit_expenses_page(const httplib::Request& req, httplib::Response& res);
void month_breakdown_expenses_graph(budget::html_writer& w, const std::string& title,
                                    budget::month month, budget::year year, bool mono = false, 
                                    const std::string& style = "");

// Overview pages
void overview_page(const httplib::Request& req, httplib::Response& res);
void overview_aggregate_all_page(const httplib::Request& req, httplib::Response& res);
void overview_aggregate_year_page(const httplib::Request& req, httplib::Response& res);
void overview_aggregate_month_page(const httplib::Request& req, httplib::Response& res);
void overview_year_page(const httplib::Request& req, httplib::Response& res);
void time_graph_savings_rate_page(const httplib::Request& req, httplib::Response& res);

// Objectives pages
void objectives_card(budget::html_writer& w);
void list_objectives_page(const httplib::Request& req, httplib::Response& res);
void status_objectives_page(const httplib::Request& req, httplib::Response& res);
void add_objectives_page(const httplib::Request& req, httplib::Response& res);
void edit_objectives_page(const httplib::Request& req, httplib::Response& res);

// Recurrings Pages
void recurrings_list_page(const httplib::Request& req, httplib::Response& res);
void add_recurrings_page(const httplib::Request& req, httplib::Response& res);
void edit_recurrings_page(const httplib::Request& req, httplib::Response& res);

// Retirement pages
void retirement_status_page(const httplib::Request& req, httplib::Response& res);
void retirement_configure_page(const httplib::Request& req, httplib::Response& res);
void retirement_fi_ratio_over_time(const httplib::Request& req, httplib::Response& res);

// Asset Pages
void assets_page(const httplib::Request& req, httplib::Response& res);
void add_assets_page(const httplib::Request& req, httplib::Response& res);
void edit_assets_page(const httplib::Request& req, httplib::Response& res);

// Asset Values Pages
void list_asset_values_page(const httplib::Request& req, httplib::Response& res);
void add_asset_values_page(const httplib::Request& req, httplib::Response& res);
void edit_asset_values_page(const httplib::Request& req, httplib::Response& res);
void full_batch_asset_values_page(const httplib::Request& req, httplib::Response& res);
void current_batch_asset_values_page(const httplib::Request& req, httplib::Response& res);

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

} //end of namespace budget
