//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>

#include "overview.hpp"
#include "accounts.hpp"

#include "writer.hpp"
#include "pages/overview_pages.hpp"
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

void budget::time_graph_savings_rate_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Savings rate over time")) {
        return;
    }

    budget::html_writer w(content_stream);

    auto ss = start_time_chart(w, "Savings rate over time", "line", "savings_time_graph", "");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, max: 100, title: { text: 'Monthly Savings Rate' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << "series: [";

    ss << "{ name: 'Savings Rate',";
    ss << "data: [";

    std::vector<float> serie;
    std::vector<std::string> dates;

    auto sy = start_year();

    for (unsigned short j = sy; j <= budget::local_day().year(); ++j) {
        budget::year year = j;

        auto sm = start_month(year);
        auto last = 13;

        if (year == budget::local_day().year()) {
            last = budget::local_day().month() + 1;
        }

        for (unsigned short i = sm; i < last; ++i) {
            budget::month month = i;

            budget::money income;
            budget::money expenses;

            for (auto& account : all_accounts(year, month)) {
                income += account.amount;
            }

            for (auto& earning : all_earnings()) {
                if (earning.date.year() == year && earning.date.month() == month) {
                    income += earning.amount;
                }
            }

            for (auto& expense : all_expenses()) {
                if (expense.date.year() == year && expense.date.month() == month) {
                    expenses += expense.amount;
                }
            }

            auto savings_rate = (income - expenses) / income;

            if (savings_rate < 0) {
                savings_rate = 0;
            }

            std::string date = "Date.UTC(" + std::to_string(year) + "," + std::to_string(month.value - 1) + ", 1)";

            serie.push_back(savings_rate);
            dates.push_back(date);

            ss << "[" << date << " ," << 100.0 * savings_rate << "],";
        }
    }

    ss << "]},";

    ss << "{ name: '12 months average',";
    ss << "data: [";

    std::array<float, 12> average_12;
    average_12.fill(0.0f);

    for (size_t i = 0; i < serie.size(); ++i) {
        average_12[i % 12] = serie[i];

        auto average = std::accumulate(average_12.begin(), average_12.end(), 0.0f);

        if (i < 12) {
            average = average / float(i + 1);
        } else {
            average = average / 12.f;
        }

        ss << "[" << dates[i] << "," << 100.0 * average << "],";
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);

    page_end(w, req, res);
}
