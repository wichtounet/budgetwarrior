//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>
#include <array>

#include "accounts.hpp"
#include "earnings.hpp"
#include "incomes.hpp"

#include "writer.hpp"
#include "pages/earnings_pages.hpp"
#include "http.hpp"
#include "config.hpp"

using namespace budget;

void budget::month_breakdown_income_graph(budget::html_writer& w, const std::string& title, budget::month month, budget::year year, bool mono, const std::string& style) {
    if (mono) {
        w.defer_script(R"=====(
            breakdown_income_colors = (function () {
                var colors = [], base = Highcharts.getOptions().colors[0], i;
                for (i = 0; i < 10; i += 1) {
                    colors.push(Highcharts.Color(base).brighten((i - 3) / 7).get());
                }
                return colors;
            }());
        )=====");
    }

    auto ss = start_chart_base(w, "pie", "month_breakdown_income_graph", style);

    ss << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    if (mono) {
        ss << R"=====(plotOptions: { pie: { dataLabels: {enabled: false},  colors: breakdown_income_colors, innerSize: '60%' }},)=====";
    }

    ss << "series: [";

    ss << "{ name: 'Income',";
    ss << "colorByPoint: true,";
    ss << "data: [";

    std::map<size_t, budget::money> account_sum;

    for (auto& earning : all_earnings_month(year, month)) {
        account_sum[earning.account] += earning.amount;
    }

    budget::money total = get_base_income();

    if (total) {
        ss << "{";
        ss << "name: 'Salary',";
        ss << "y: " << budget::to_flat_string(total);
        ss << "},";
    }

    for (auto& sum : account_sum) {
        ss << "{";
        ss << "name: '" << get_account(sum.first).name << "',";
        ss << "y: " << budget::to_flat_string(sum.second);
        ss << "},";

        total += sum.second;
    }

    ss << "]},";

    ss << "],";

    if (mono) {
        ss << R"=====(title: {verticalAlign: 'middle', useHTML: true, text: ')=====";

        ss << R"=====(<div class="gauge-cash-flow-title"><strong>)=====";
        ss << title;
        ss << R"=====(</strong><br/><hr class="flat-hr" />)=====";

        ss << R"=====(<span class="text-success">)=====";
        ss << total << " __currency__";
        ss << R"=====(</span></div>)=====";
        ss << R"=====('},)=====";
    } else {
        ss << R"=====(title: {text: ')=====";
        ss << title;
        ss << R"=====('},)=====";
    }

    end_chart(w, ss);
}


void budget::time_graph_income_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Income over time")) {
        return;
    }

    budget::html_writer w(content_stream);

    auto ss = start_time_chart(w, "Income over time", "line", "income_time_graph", "");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Monthly Income' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << "series: [";

    ss << "{ name: 'Monthly income',";
    ss << "data: [";

    std::vector<budget::money> serie;
    std::vector<std::string> dates;

    auto sy = start_year();

    for(unsigned short j = sy; j <= budget::local_day().year(); ++j){
        budget::year year = j;

        auto sm = start_month(year);
        auto last = 13;

        if(year == budget::local_day().year()){
            last = budget::local_day().month() + 1;
        }

        for(unsigned short i = sm; i < last; ++i){
            budget::month month = i;

            budget::money sum = get_base_income(budget::date(year, month, 2));

            for (auto& earning : all_earnings()) {
                if (earning.date.year() == year && earning.date.month() == month) {
                    sum += earning.amount;
                }
            }

            std::string date = "Date.UTC(" + std::to_string(year) + "," + std::to_string(month.value - 1) + ", 1)";

            serie.push_back(sum);
            dates.push_back(date);

            ss << "[" << date <<  "," << budget::to_flat_string(sum) << "],";
        }
    }

    ss << "]},";

    ss << "{ name: '12 months average',";
    ss << "data: [";

    std::array<budget::money, 12> average_12;

    for(size_t i = 0; i < serie.size(); ++i){
        average_12[i % 12] = serie[i];

        auto average = std::accumulate(average_12.begin(), average_12.end(), budget::money());

        if(i < 12){
            average = average / int(i + 1);
        } else {
            average = average / 12;
        }

        ss << "[" << dates[i] << "," << budget::to_flat_string(average) << "],";
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);

    page_end(w, req, res);
}

void budget::time_graph_earnings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Earnings over time")) {
        return;
    }

    budget::html_writer w(content_stream);

    auto ss = start_time_chart(w, "Earnings over time", "line", "earnings_time_graph", "");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Monthly Earnings' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << "series: [";

    ss << "{ name: 'Monthly earnings',";
    ss << "data: [";

    auto sy = start_year();

    for(unsigned short j = sy; j <= budget::local_day().year(); ++j){
        budget::year year = j;

        auto sm = start_month(year);
        auto last = 13;

        if(year == budget::local_day().year()){
            last = budget::local_day().month() + 1;
        }

        for(unsigned short i = sm; i < last; ++i){
            budget::month month = i;

            budget::money sum;

            for(auto& earning : all_earnings()){
                if(earning.date.year() == year && earning.date.month() == month){
                    sum += earning.amount;
                }
            }

            ss << "[Date.UTC(" << year << "," << month.value - 1 << ", 1) ," << budget::to_flat_string(sum) << "],";
        }
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);

    page_end(w, req, res);
}

void budget::add_earnings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "New earning")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "New earning" << title_end;

    form_begin(w, "/api/earnings/add/", "/earnings/add/");

    add_date_picker(w);
    add_name_picker(w);
    add_amount_picker(w);

    std::string account;

    if (config_contains("default_account")) {
        auto default_account = config_value("default_account");

        if (account_exists(default_account)) {
            auto today = budget::local_day();
            account = budget::to_string(get_account(default_account, today.year(), today.month()).id);
        }
    }

    add_account_picker(w, budget::local_day(), account);

    form_end(w);

    page_end(w, req, res);
}

void budget::edit_earnings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Edit earning")) {
        return;
    }

    budget::html_writer w(content_stream);

    if (!req.has_param("input_id") || !req.has_param("back_page")) {
        display_error_message(w, "Invalid parameter for the request");
    } else {
        auto input_id = req.get_param_value("input_id");

        if (!earning_exists(budget::to_number<size_t>(input_id))) {
            display_error_message(w, "The earning " + input_id + " does not exist");
        } else {
            auto back_page = req.get_param_value("back_page");

            w << title_begin << "Edit earning " << input_id << title_end;

            form_begin_edit(w, "/api/earnings/edit/", back_page, input_id);

            auto& earning = earning_get(budget::to_number<size_t>(input_id));

            add_date_picker(w, budget::to_string(earning.date));
            add_name_picker(w, earning.name);
            add_amount_picker(w, budget::to_flat_string(earning.amount));
            add_account_picker(w, earning.date, budget::to_string(earning.account));

            form_end(w);
        }
    }

    page_end(w, req, res);
}

void budget::earnings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Earnings")) {
        return;
    }

    budget::html_writer w(content_stream);

    if (req.matches.size() == 3) {
        show_earnings(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        show_earnings(w);
    }

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::all_earnings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "All Earnings")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_all_earnings(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::search_earnings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Search Earnings")) {
        return;
    }

    budget::html_writer w(content_stream);

    page_form_begin(w, "/earnings/search/");

    add_name_picker(w);

    form_end(w);

    if(req.has_param("input_name")){
        auto search = req.get_param_value("input_name");

        search_earnings(search, w);
    }

    make_tables_sortable(w);

    page_end(w, req, res);
}
