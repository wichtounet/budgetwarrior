//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "recurring.hpp"

#include "writer.hpp"
#include "server_pages.hpp"
#include "http.hpp"

using namespace budget;

void budget::recurrings_list_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Recurrings List")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_recurrings(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::add_recurrings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "New Recurring Expense")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "New Recurring Expense" << title_end;

    form_begin(w, "/api/recurrings/add/", "/recurrings/add/");

    add_name_picker(w);
    add_amount_picker(w);
    add_account_picker(w, budget::local_day());

    form_end(w);

    page_end(w, req, res);
}

void budget::edit_recurrings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Edit Recurring Expense")) {
        return;
    }

    budget::html_writer w(content_stream);

    if (!req.has_param("input_id") || !req.has_param("back_page")) {
        display_error_message(w, "Invalid parameter for the request");
    } else {
        auto input_id = req.get_param_value("input_id");

        if (!recurring_exists(budget::to_number<size_t>(input_id))) {
            display_error_message(w, "The recurring expense " + input_id + " does not exist");
        } else {
            auto back_page = req.get_param_value("back_page");

            w << title_begin << "Edit Recurring Expense " << input_id << title_end;

            form_begin_edit(w, "/api/recurrings/edit/", back_page, input_id);

            auto& recurring = recurring_get(budget::to_number<size_t>(input_id));

            add_name_picker(w, recurring.name);
            add_amount_picker(w, budget::to_flat_string(recurring.amount));
            add_account_picker(w, budget::local_day(), budget::to_string(recurring.account));

            form_end(w);
        }
    }

    page_end(w, req, res);
}
