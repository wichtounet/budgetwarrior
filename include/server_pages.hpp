//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <string>
#include <vector>

namespace httplib {
struct Server;
struct Response;
struct Request;
};

namespace budget {

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

void add_name_picker(budget::writer& w, const std::string& default_value = "");
void add_title_picker(budget::writer& w, const std::string& default_value = "");
void add_amount_picker(budget::writer& w, const std::string& default_value = "");
void add_paid_amount_picker(budget::writer& w, const std::string& default_value = "");
void add_paid_picker(budget::writer& w, bool paid);

// Pages

// Debts Pages
void list_debts_page(const httplib::Request& req, httplib::Response& res);
void all_debts_page(const httplib::Request& req, httplib::Response& res);
void edit_debts_page(const httplib::Request& req, httplib::Response& res);
void add_debts_page(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
