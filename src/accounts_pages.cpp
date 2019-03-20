//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "accounts.hpp"

#include "writer.hpp"
#include "server_pages.hpp"
#include "http.hpp"

using namespace budget;

void budget::accounts_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Accounts")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_accounts(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::all_accounts_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "All Accounts")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_all_accounts(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::add_accounts_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "New account")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "New account" << title_end;

    form_begin(w, "/api/accounts/add/", "/accounts/add/");

    add_name_picker(w);
    add_amount_picker(w);

    form_end(w);

    page_end(w, req, res);
}

void budget::edit_accounts_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Edit account")) {
        return;
    }

    budget::html_writer w(content_stream);

    if (!req.has_param("input_id") || !req.has_param("back_page")) {
        display_error_message(w, "Invalid parameter for the request");
    } else {
        auto input_id = req.get_param_value("input_id");

        if (!account_exists(budget::to_number<size_t>(input_id))) {
            display_error_message(w, "The account " + input_id + " does not exist");
        } else {
            auto back_page = req.get_param_value("back_page");

            w << title_begin << "Edit account " << input_id << title_end;

            form_begin_edit(w, "/api/accounts/edit/", back_page, input_id);

            auto& account = account_get(budget::to_number<size_t>(input_id));

            add_name_picker(w, account.name);
            add_amount_picker(w, budget::to_flat_string(account.amount));

            form_end(w);
        }
    }

    page_end(w, req, res);
}

void budget::archive_accounts_month_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Archive accounts from the beginning of the month")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "Archive accounts from the beginning of the month" << title_end;

    form_begin(w, "/api/accounts/archive/month/", "/accounts/");

    w << "<p>This will create new accounts that will be used starting from the beginning of the current month. Are you sure you want to proceed ? </p>";

    form_end(w, "Confirm");

    page_end(w, req, res);
}

void budget::archive_accounts_year_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Archive accounts from the beginning of the year")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "Archive accounts from the beginning of the year" << title_end;

    form_begin(w, "/api/accounts/archive/year/", "/accounts/");

    w << "<p>This will create new accounts that will be used starting from the beginning of the current year. Are you sure you want to proceed ? </p>";

    form_end(w, "Confirm");

    page_end(w, req, res);
}
