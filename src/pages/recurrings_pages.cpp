//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "recurring.hpp"

#include "writer.hpp"
#include "pages/recurrings_pages.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void add_frequencypicker(budget::writer& w, const std::string& default_value = "") {
    w << R"=====(
            <div class="form-group">
                <label for="input_recurs">Recurrence</label>
                <select class="form-control" id="input_recurs" name="input_recurs">
    )=====";

    if ("monthly" == default_value) {
        w << "<option selected value=\"monthly\">Monthly</option>";
    } else {
        w << "<option value=\"monthly\">Monthly</option>";
    }

    if ("weekly" == default_value) {
        w << "<option selected value=\"weekly\">Weekly</option>";
    } else {
        w << "<option value=\"weekly\">Weekly</option>";
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void add_type_picker(budget::writer& w, const std::string& default_value = "") {
    w << R"=====(
            <div class="form-group">
                <label for="input_type">Type</label>
                <select class="form-control" id="input_type" name="input_type">
    )=====";

    if ("expense" == default_value) {
        w << "<option selected value=\"expense\">Expense</option>";
    } else {
        w << "<option value=\"expense\">Expense</option>";
    }

    if ("earning" == default_value) {
        w << "<option selected value=\"earning\">Earning</option>";
    } else {
        w << "<option value=\"earning\">Earning</option>";
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

} // namespace

void budget::recurrings_list_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Recurring Operations")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_recurrings(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::add_recurrings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "New Recurring Operation")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "New Recurring Expense" << title_end;

    form_begin(w, "/api/recurrings/add/", "/recurrings/add/");

    add_name_picker(w);
    add_amount_picker(w);
    add_account_picker(w, budget::local_day());
    add_frequencypicker(w);
    add_type_picker(w);

    form_end(w);

    page_end(w, req, res);
}

void budget::edit_recurrings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Edit Recurring Operation")) {
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

            auto recurring = recurring_get(budget::to_number<size_t>(input_id));

            add_name_picker(w, recurring.name);
            add_amount_picker(w, budget::money_to_string(recurring.amount));
            add_account_picker(w, budget::local_day(), budget::to_string(recurring.account));
            add_frequencypicker(w, recurring.recurs);

            form_end(w);
        }
    }

    page_end(w, req, res);
}
