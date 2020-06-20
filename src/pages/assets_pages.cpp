//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "assets.hpp"

#include "writer.hpp"
#include "pages/assets_pages.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void add_currency_picker(budget::writer& w, const std::string& default_value = "") {
    add_text_picker(w, "Currency", "input_currency", default_value);
}

void add_portfolio_picker(budget::writer& w, bool portfolio) {
    add_yes_no_picker(w, "Part of the portfolio", "input_portfolio", portfolio);
}

void add_share_based_picker(budget::writer& w, bool share_based) {
    add_yes_no_picker(w, "Using shares?", "input_share_based", share_based);
}

} // namespace

void budget::assets_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Assets")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_assets(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::add_assets_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "New asset")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "New asset" << title_end;

    form_begin(w, "/api/assets/add/", "/assets/add/");

    add_name_picker(w);

    for (auto & clas : all_asset_classes()) {
        add_money_picker(w, clas.name + " (%)", "input_class_" + to_string(clas.id), "");
    }

    add_currency_picker(w);
    add_portfolio_picker(w, false);
    add_money_picker(w, "Percent of portfolio (%)", "input_alloc", "");
    add_share_based_picker(w, false);
    add_text_picker(w, "Ticker", "input_ticker", "");

    form_end(w);

    page_end(w, req, res);
}

void budget::edit_assets_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Edit asset")) {
        return;
    }

    budget::html_writer w(content_stream);

    if (!req.has_param("input_id") || !req.has_param("back_page")) {
        display_error_message(w, "Invalid parameter for the request");
    } else {
        auto input_id = req.get_param_value("input_id");

        if (!asset_exists(budget::to_number<size_t>(input_id))) {
            display_error_message(w, "The asset " + input_id + " does not exist");
        } else {
            auto back_page = req.get_param_value("back_page");

            w << title_begin << "Edit asset " << input_id << title_end;

            form_begin_edit(w, "/api/assets/edit/", back_page, input_id);

            auto& asset = asset_get(budget::to_number<size_t>(input_id));

            add_name_picker(w, asset.name);

            for (auto & clas : all_asset_classes()) {
                add_money_picker(w, clas.name + " (%)", "input_class_" + to_string(clas.id), budget::to_flat_string(get_asset_class_allocation(asset, clas)));
            }

            add_currency_picker(w, asset.currency);
            add_portfolio_picker(w, asset.portfolio);
            add_money_picker(w, "Percent of portfolio (%)", "input_alloc", budget::to_flat_string(asset.portfolio_alloc));
            add_share_based_picker(w, asset.share_based);
            add_text_picker(w, "Ticker", "input_ticker", asset.ticker);

            form_end(w);
        }
    }

    page_end(w, req, res);
}

