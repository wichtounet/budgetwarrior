//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "retirement.hpp"
#include "config.hpp"
#include "assets.hpp"

#include "writer.hpp"
#include "pages/retirement_pages.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void add_percent_picker(budget::writer& w, const std::string& title, const std::string& name, double default_value = 0.0) {
    w << R"=====(<div class="form-group">)=====";

    w << "<label for=\"" << name << "\">" << title << "</label>";
    w << "<input required type=\"number\" min=\"0\" max=\"100\" step=\"0.01\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    w << " value=\"" << default_value << "\" ";
    w << R"=====(
            >
         </div>
    )=====";
}

} // namespace

void budget::retirement_status_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Retirement status")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "Retirement status" << title_end;

    if(!internal_config_contains("withdrawal_rate")){
        display_error_message(w, "Not enough information, please configure Retirement Options first");
        page_end(w, req, res);
        return;
    }

    if(!internal_config_contains("expected_roi")){
        display_error_message(w, "Not enough information, please configure Retirement Options first");
        page_end(w, req, res);
        return;
    }

    budget::retirement_status(w);

    page_end(w, req, res);
}

void budget::retirement_configure_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Retirement configure")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "Retirement Options" << title_end;

    form_begin(w, "/api/retirement/configure/", "/retirement/status/");

    if (!internal_config_contains("withdrawal_rate")) {
        add_percent_picker(w, "Withdrawal Rate [%]", "input_wrate", 4.0);
    } else {
        add_percent_picker(w, "Withdrawal Rate [%]", "input_wrate", to_number<double>(internal_config_value("withdrawal_rate")));
    }

    if (!internal_config_contains("expected_roi")) {
        add_percent_picker(w, "Annual Return [%]", "input_roi", 5.0);
    } else {
        add_percent_picker(w, "Annual Return [%]", "input_roi", to_number<double>(internal_config_value("expected_roi")));
    }

    form_end(w);

    page_end(w, req, res);
}

void budget::retirement_fi_ratio_over_time(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "FI Ratio over time")) {
        return;
    }

    budget::html_writer w(content_stream);

    if (all_assets().empty() || all_asset_values().empty()){
        page_end(w, req, res);
        return;
    }

    auto ss = start_time_chart(w, "FI Ratio over time", "line", "fi_time_graph", "");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'FI Ratio' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << "series: [";

    ss << "{ name: 'FI Ratio %',";
    ss << "data: [";

    std::vector<budget::money> serie;
    std::vector<std::string> dates;

    auto date     = budget::asset_start_date();
    auto end_date = budget::local_day();

    auto asset_values = all_asset_values();

    while (date <= end_date) {
        auto ratio = budget::fi_ratio(date, asset_values);

        std::string datestr = "Date.UTC(" + std::to_string(date.year()) + "," + std::to_string(date.month().value - 1) + ", " + std::to_string(date.day().value) + ")";
        ss << "[" << datestr << "," << budget::to_string(100 * ratio) << "],";

        date += days(1);
    }

    ss << "]},";
    ss << "]";

    end_chart(w, ss);

    page_end(w, req, res);
}
