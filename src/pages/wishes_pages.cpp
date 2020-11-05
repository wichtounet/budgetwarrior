//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "wishes.hpp"

#include "writer.hpp"
#include "pages/wishes_pages.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void add_importance_picker(budget::writer& w, int importance) {
    w << R"=====(
            <div class="form-group">
                <label for="input_importance">Importance</label>
                <select class="form-control" id="input_importance" name="input_importance">
    )=====";

    if (importance == 1) {
        w << "<option selected value=\"1\">Low</option>";
    } else {
        w << "<option value=\"1\">Low</option>";
    }

    if (importance == 2) {
        w << "<option selected value=\"2\">Medium</option>";
    } else {
        w << "<option value=\"2\">Medium</option>";
    }

    if (importance == 3) {
        w << "<option selected value=\"3\">High</option>";
    } else {
        w << "<option value=\"3\">High</option>";
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void add_urgency_picker(budget::writer& w, int urgency) {
    w << R"=====(
            <div class="form-group">
                <label for="input_urgency">Urgency</label>
                <select class="form-control" id="input_urgency" name="input_urgency">
    )=====";

    if (urgency == 1) {
        w << "<option selected value=\"1\">Low</option>";
    } else {
        w << "<option value=\"1\">Low</option>";
    }

    if (urgency == 2) {
        w << "<option selected value=\"2\">Medium</option>";
    } else {
        w << "<option value=\"2\">Medium</option>";
    }

    if (urgency == 3) {
        w << "<option selected value=\"3\">High</option>";
    } else {
        w << "<option value=\"3\">High</option>";
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

} // namespace

void budget::wishes_list_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Objectives List")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::list_wishes(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::wishes_status_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Objectives Status")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::status_wishes(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::wishes_estimate_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Objectives Status")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::estimate_wishes(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::add_wishes_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "New Wish")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "New Wish" << title_end;

    form_begin(w, "/api/wishes/add/", "/wishes/add/");

    add_name_picker(w);
    add_importance_picker(w, 2);
    add_urgency_picker(w, 2);
    add_amount_picker(w);

    form_end(w);

    page_end(w, req, res);
}

void budget::edit_wishes_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;

    if (!page_get_start(req, res, content_stream, "Edit Wish", {"input_id", "back_page"})){
        return;
    }

    budget::html_writer w(content_stream);

    auto input_id = req.get_param_value("input_id");

    if (!wish_exists(budget::to_number<size_t>(input_id))) {
        display_error_message(w, "The wish " + input_id + " does not exist");
    } else {
        auto back_page = req.get_param_value("back_page");

        w << title_begin << "Edit wish " << input_id << title_end;

        form_begin_edit(w, "/api/wishes/edit/", back_page, input_id);

        auto wish = wish_get(budget::to_number<size_t>(input_id));

        add_name_picker(w, wish.name);
        add_importance_picker(w, wish.importance);
        add_urgency_picker(w, wish.urgency);
        add_amount_picker(w, budget::money_to_string(wish.amount));
        add_paid_picker(w, wish.paid);
        add_paid_amount_picker(w, budget::money_to_string(wish.paid_amount));

        form_end(w);
    }

    page_end(w, req, res);
}
