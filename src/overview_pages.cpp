//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>

#include "overview.hpp"

#include "writer.hpp"
#include "server_pages.hpp"
#include "http.hpp"
#include "config.hpp"

using namespace budget;

void budget::overview_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Overview")) {
        return;
    }

    budget::html_writer w(content_stream);

    if (req.matches.size() == 3) {
        display_month_overview(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        display_month_overview(w);
    }

    page_end(w, req, res);
}

void budget::overview_aggregate_all_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Overview Aggregate")) {
        return;
    }

    budget::html_writer w(content_stream);

    // Configuration of the overview
    bool full             = config_contains_and_true("aggregate_full");
    bool disable_groups   = config_contains_and_true("aggregate_no_group");
    std::string separator = config_value("aggregate_separator", "/");

    aggregate_all_overview(w, full, disable_groups, separator);

    page_end(w, req, res);
}

void budget::overview_aggregate_year_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Overview Aggregate")) {
        return;
    }

    budget::html_writer w(content_stream);

    // Configuration of the overview
    bool full             = config_contains_and_true("aggregate_full");
    bool disable_groups   = config_contains_and_true("aggregate_no_group");
    std::string separator = config_value("aggregate_separator", "/");

    if (req.matches.size() == 2) {
        aggregate_year_overview(w, full, disable_groups, separator, to_number<size_t>(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_year_overview(w, full, disable_groups, separator, today.year());
    }

    page_end(w, req, res);
}

void budget::overview_aggregate_month_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Overview Aggregate")) {
        return;
    }

    budget::html_writer w(content_stream);

    // Configuration of the overview
    bool full             = config_contains_and_true("aggregate_full");
    bool disable_groups   = config_contains_and_true("aggregate_no_group");
    std::string separator = config_value("aggregate_separator", "/");

    if (req.matches.size() == 3) {
        aggregate_month_overview(w, full, disable_groups, separator, to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_month_overview(w, full, disable_groups, separator, today.month(), today.year());
    }

    page_end(w, req, res);
}

void budget::overview_year_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Overview Year")) {
        return;
    }

    budget::html_writer w(content_stream);

    if (req.matches.size() == 2) {
        display_year_overview(to_number<size_t>(req.matches[1]), w);
    } else {
        display_year_overview(w);
    }

    page_end(w, req, res);
}
