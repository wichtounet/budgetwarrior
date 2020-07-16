//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "liabilities.hpp"

#include "writer.hpp"
#include "pages/liabilities_pages.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void add_currency_picker(budget::writer& w, const std::string& default_value = "") {
    add_text_picker(w, "Currency", "input_currency", default_value);
}

} // namespace

void budget::list_liabilities_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "List liabilities")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_liabilities(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::add_liabilities_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "New liability")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "New liability" << title_end;

    form_begin(w, "/api/liabilities/add/", "/liabilities/add/");

    add_name_picker(w);
    add_currency_picker(w);

    form_end(w);

    page_end(w, req, res);
}

void budget::edit_liabilities_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;

    if (!page_get_start(req, res, content_stream, "Edit liability", {"input_id", "back_page"})){
        return;
    }

    budget::html_writer w(content_stream);

    auto input_id = req.get_param_value("input_id");
    auto id = budget::to_number<size_t>(input_id);

    if (!liability_exists(id)) {
        display_error_message(w, "The liability " + input_id + " does not exist");
    } else {
        auto back_page = req.get_param_value("back_page");

        w << title_begin << "Edit liability " << input_id << title_end;

        form_begin_edit(w, "/api/liabilities/edit/", back_page, input_id);

        auto& liability = liability_get(id);

        add_name_picker(w, liability.name);
        add_currency_picker(w, liability.currency);

        form_end(w);
    }

    page_end(w, req, res);
}
