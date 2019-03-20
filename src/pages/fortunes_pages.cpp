//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "fortune.hpp"

#include "pages/fortunes_pages.hpp"
#include "writer.hpp"
#include "http.hpp"

using namespace budget;

void budget::list_fortunes_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Objectives List")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::list_fortunes(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::graph_fortunes_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Fortune")) {
        return;
    }

    budget::html_writer w(content_stream);

    auto ss = start_chart(w, "Fortune", "spline");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Fortune' }},)=====";

    ss << "series: [";

    ss << "{ name: 'Fortune',";
    ss << "data: [";

    auto sorted_fortunes = all_fortunes();

    std::sort(sorted_fortunes.begin(), sorted_fortunes.end(),
              [](const budget::fortune& a, const budget::fortune& b) { return a.check_date < b.check_date; });

    for (auto& value : sorted_fortunes) {
        auto& date = value.check_date;

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(value.amount) << "],";
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);

    page_end(w, req, res);
}

void budget::status_fortunes_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Objectives Status")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::status_fortunes(w, false);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::add_fortunes_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "New fortune")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "New fortune" << title_end;

    form_begin(w, "/api/fortunes/add/", "/fortunes/add/");

    add_date_picker(w);
    add_amount_picker(w);

    form_end(w);

    page_end(w, req, res);
}

void budget::edit_fortunes_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;

    if (!page_get_start(req, res, content_stream, "Edit Fortune", {"input_id", "back_page"})){
        return;
    }

    budget::html_writer w(content_stream);

    auto input_id = req.get_param_value("input_id");

    if (!fortune_exists(budget::to_number<size_t>(input_id))) {
        display_error_message(w, "The fortune " + input_id + " does not exist");
    } else {
        auto back_page = req.get_param_value("back_page");

        w << title_begin << "Edit fortune " << input_id << title_end;

        form_begin_edit(w, "/api/fortunes/edit/", back_page, input_id);

        auto& fortune = fortune_get(budget::to_number<size_t>(input_id));

        add_date_picker(w, budget::to_string(fortune.check_date));
        add_amount_picker(w, budget::to_flat_string(fortune.amount));

        form_end(w);
    }

    page_end(w, req, res);
}
