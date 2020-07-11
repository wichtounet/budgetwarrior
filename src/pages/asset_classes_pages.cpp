//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "assets.hpp"

#include "writer.hpp"
#include "pages/asset_classes_pages.hpp"
#include "http.hpp"

using namespace budget;

void budget::list_asset_classes_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "List asset classes")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_asset_classes(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::add_asset_classes_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "New asset class")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "New asset class" << title_end;

    form_begin(w, "/api/asset_classes/add/", "/asset_classes/add/");

    add_name_picker(w);

    form_end(w);

    page_end(w, req, res);
}

void budget::edit_asset_classes_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;

    if (!page_get_start(req, res, content_stream, "Edit asset class", {"input_id", "back_page"})){
        return;
    }

    budget::html_writer w(content_stream);

    auto input_id = req.get_param_value("input_id");
    auto id = budget::to_number<size_t>(input_id);

    if (!asset_class_exists(id)) {
        display_error_message(w, "The asset class " + input_id + " does not exist");
    } else {
        auto back_page = req.get_param_value("back_page");

        w << title_begin << "Edit asset class " << input_id << title_end;

        form_begin_edit(w, "/api/asset_classes/edit/", back_page, input_id);

        auto& asset_class = asset_class_get(id);

        add_name_picker(w, asset_class.name);

        form_end(w);
    }

    page_end(w, req, res);
}
