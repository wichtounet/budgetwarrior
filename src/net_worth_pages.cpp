//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "cpp_utils/assert.hpp"

#include "assets.hpp"

#include "writer.hpp"
#include "server_pages.hpp"
#include "http.hpp"
#include "currency.hpp"
#include "config.hpp"

using namespace budget;

void budget::assets_card(budget::html_writer& w){
    w << R"=====(<div class="card">)=====";

    w << R"=====(<div class="card-header card-header-primary">)=====";
    w << R"=====(<div>Assets</div>)=====";
    w << R"=====(</div>)====="; // card-header

    w << R"=====(<div class="card-body">)=====";

    std::string separator = "/";
    if (budget::config_contains("aggregate_separator")) {
        separator = budget::config_value("aggregate_separator");
    }

    // If all assets are in the form group/asset, then we use special style

    bool group_style = true;

    if (budget::config_contains("asset_no_group")) {
        if (budget::config_value("asset_no_group") == "true") {
            group_style = false;
        }
    }

    if (group_style) {
        for (auto& asset : all_user_assets()) {
            auto pos = asset.name.find(separator);
            if (pos == 0 || pos == std::string::npos) {
                group_style = false;
                break;
            }
        }
    }

    if (group_style) {
        std::vector<std::string> groups;

        for (auto& asset : all_user_assets()) {
            std::string group = asset.name.substr(0, asset.name.find(separator));

            if (std::find(groups.begin(), groups.end(), group) == groups.end()) {
                groups.push_back(group);
            }
        }

        for (auto& group : groups) {
            bool started = false;

            for (auto& asset : all_user_assets()) {
                if (asset.name.substr(0, asset.name.find(separator)) == group) {
                    auto short_name = asset.name.substr(asset.name.find(separator) + 1);

                    auto amount = get_asset_value(asset);

                    if (amount) {
                        if (!started) {
                            w << "<div class=\"asset_group\">";
                            w << group;
                            w << "</div>";

                            started = true;
                        }

                        w << R"=====(<div class="asset_row row">)=====";
                        w << R"=====(<div class="asset_name col-md-8 col-xl-9 small">)=====";
                        w << short_name;
                        w << R"=====(</div>)=====";
                        w << R"=====(<div class="asset_right col-md-4 col-xl-3 text-right small">)=====";
                        w << R"=====(<span class="asset_amount">)=====";
                        w << budget::to_string(amount) << " " << asset.currency;
                        w << R"=====(</span>)=====";
                        w << R"=====(<br />)=====";
                        w << R"=====(</div>)=====";
                        w << R"=====(</div>)=====";
                    }
                }
            }
        }
    } else {
        bool first = true;

        for (auto& asset : all_user_assets()) {
            auto amount = get_asset_value(asset);

            if (amount) {
                if (!first) {
                    w << R"=====(<hr />)=====";
                }

                w << R"=====(<div class="row">)=====";
                w << R"=====(<div class="col-md-8 col-xl-9 small">)=====";
                w << asset.name;
                w << R"=====(</div>)=====";
                w << R"=====(<div class="col-md-4 col-xl-3 text-right small">)=====";
                w << budget::to_string(amount) << " " << asset.currency;
                w << R"=====(<br />)=====";
                w << R"=====(</div>)=====";
                w << R"=====(</div>)=====";

                first = false;
            }
        }
    }

    w << R"=====(</div>)====="; //card-body
    w << R"=====(</div>)====="; //card
}

void budget::net_worth_graph(budget::html_writer& w, const std::string style, bool card) {
    // if the user does not use assets, this graph does not make sense
    if(all_assets().empty() || all_asset_values().empty()){
        return;
    }

    if (card) {
        w << R"=====(<div class="card">)=====";

        w << R"=====(<div class="card-header card-header-primary">)=====";
        w << R"=====(<div class="float-left">Net Worth</div>)=====";
        w << R"=====(<div class="float-right">)=====";
        w << get_net_worth() << " __currency__";
        w << R"=====(</div>)=====";
        w << R"=====(<div class="clearfix"></div>)=====";
        w << R"=====(</div>)====="; // card-header

        w << R"=====(<div class="card-body">)=====";
    }

    auto ss = start_time_chart(w, card ? "" : "Net worth", "area", "net_worth_graph", style);

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    if (!card) {
        ss << R"=====(subtitle: {)=====";
        ss << "text: '" << get_net_worth() << " __currency__',";
        ss << R"=====(floating:true, align:"right", verticalAlign: "top", style: { fontWeight: "bold", fontSize: "inherit" })=====";
        ss << R"=====(},)=====";
    }

    ss << "series: [";

    ss << "{ name: 'Net Worth',";
    ss << "data: [";

    std::map<size_t, budget::money> asset_amounts;

    auto sorted_asset_values = all_sorted_asset_values();

    auto it  = sorted_asset_values.begin();
    auto end = sorted_asset_values.end();

    while (it != end) {
        auto date = it->set_date;

        while (it->set_date == date) {
            asset_amounts[it->asset_id] = it->amount * exchange_rate(get_asset(it->asset_id).currency, date);

            ++it;
        }

        budget::money sum;

        for (auto& asset : asset_amounts) {
            sum += asset.second;
        }

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);

    if (card) {
        w << R"=====(</div>)====="; //card-body
        w << R"=====(</div>)====="; //card
    }
}

void budget::net_worth_status_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Net Worth Status")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_asset_values(w);

    page_end(w, req, res);
}

void budget::net_worth_small_status_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Net Worth Status")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::small_show_asset_values(w);

    page_end(w, req, res);
}

void budget::net_worth_graph_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Net Worth Graph")) {
        return;
    }

    budget::html_writer w(content_stream);

    net_worth_graph(w);

    page_end(w, req, res);
}

void budget::net_worth_allocation_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Net Worth Allocation")) {
        return;
    }

    std::vector<std::string> names{"Int. Stocks", "Dom. Stocks", "Bonds", "Cash"};

    budget::html_writer w(content_stream);

    // 1. Display the currency breakdown over time

    auto ss = start_time_chart(w, "Net worth allocation", "area", "allocation_time_graph");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(tooltip: {split: true},)=====";
    ss << R"=====(plotOptions: {area: {stacking: 'percent'}},)=====";

    ss << "series: [";

    auto sorted_asset_values = all_sorted_asset_values();

    for(size_t i = 0; i < names.size(); ++i){
        ss << "{ name: '" << names[i] << "',";
        ss << "data: [";

        std::map<size_t, budget::money> asset_amounts;

        auto it  = sorted_asset_values.begin();
        auto end = sorted_asset_values.end();

        while (it != end) {
            auto date = it->set_date;

            while (it->set_date == date) {
                auto& asset = get_asset(it->asset_id);

                auto amount = it->amount * exchange_rate(asset.currency, date);

                if(i == 0 && asset.int_stocks){
                    asset_amounts[it->asset_id] = amount * (float(asset.int_stocks) / 100.0f);
                }

                if(i == 1 && asset.dom_stocks){
                    asset_amounts[it->asset_id] = amount * (float(asset.dom_stocks) / 100.0f);
                }

                if(i == 2 && asset.bonds){
                    asset_amounts[it->asset_id] = amount * (float(asset.bonds) / 100.0f);
                }

                if(i == 3 && asset.cash){
                    asset_amounts[it->asset_id] = amount * (float(asset.cash) / 100.0f);
                }

                ++it;
            }

            budget::money sum;

            for (auto& asset : asset_amounts) {
                sum += asset.second;
            }

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";
        }

        ss << "]},";
    }

    ss << "]";

    end_chart(w, ss);

    // 2. Display the current currency breakdown

    auto ss2 = start_chart(w, "Current Allocation Breakdown", "pie", "allocation_breakdown_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Classes',";
    ss2 << "colorByPoint: true,";
    ss2 << "data: [";

    for(size_t i = 0; i < names.size(); ++i){
        ss2 << "{ name: '" << names[i] << "',";
        ss2 << "y: ";

        std::map<size_t, budget::money> asset_amounts;

        for (auto& asset_value : sorted_asset_values) {
            auto& asset = get_asset(asset_value.asset_id);

            auto amount = asset_value.amount * exchange_rate(asset.currency);

            if(i == 0 && asset.int_stocks){
                asset_amounts[asset_value.asset_id] = amount * (float(asset.int_stocks) / 100.0f);
            }

            if(i == 1 && asset.dom_stocks){
                asset_amounts[asset_value.asset_id] = amount * (float(asset.dom_stocks) / 100.0f);
            }

            if(i == 2 && asset.bonds){
                asset_amounts[asset_value.asset_id] = amount * (float(asset.bonds) / 100.0f);
            }

            if(i == 3 && asset.cash){
                asset_amounts[asset_value.asset_id] = amount * (float(asset.cash) / 100.0f);
            }
        }

        budget::money sum;

        for (auto& asset : asset_amounts) {
            sum += asset.second;
        }

        ss2 << budget::to_flat_string(sum);

        ss2 << "},";
    }

    ss2 << "]},";

    ss2 << "]";

    end_chart(w, ss2);

    page_end(w, req, res);
}

void budget::portfolio_allocation_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Portfolio Allocation")) {
        return;
    }

    std::vector<std::string> names{"Int. Stocks", "Dom. Stocks", "Bonds", "Cash"};

    budget::html_writer w(content_stream);

    // 1. Display the currency breakdown over time

    auto ss = start_time_chart(w, "Portfolio allocation", "area", "allocation_time_graph");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(tooltip: {split: true},)=====";
    ss << R"=====(plotOptions: {area: {stacking: 'percent'}},)=====";

    ss << "series: [";

    auto sorted_asset_values = all_sorted_asset_values();

    for(size_t i = 0; i < names.size(); ++i){
        ss << "{ name: '" << names[i] << "',";
        ss << "data: [";

        std::map<size_t, budget::money> asset_amounts;

        auto it  = sorted_asset_values.begin();
        auto end = sorted_asset_values.end();

        while (it != end) {
            auto date = it->set_date;

            while (it->set_date == date) {
                auto& asset = get_asset(it->asset_id);

                if(asset.portfolio){
                    auto amount = it->amount * exchange_rate(asset.currency, date);

                    if(i == 0 && asset.int_stocks){
                        asset_amounts[it->asset_id] = amount * (float(asset.int_stocks) / 100.0f);
                    }

                    if(i == 1 && asset.dom_stocks){
                        asset_amounts[it->asset_id] = amount * (float(asset.dom_stocks) / 100.0f);
                    }

                    if(i == 2 && asset.bonds){
                        asset_amounts[it->asset_id] = amount * (float(asset.bonds) / 100.0f);
                    }

                    if(i == 3 && asset.cash){
                        asset_amounts[it->asset_id] = amount * (float(asset.cash) / 100.0f);
                    }
                }

                ++it;
            }

            budget::money sum;

            for (auto& asset : asset_amounts) {
                sum += asset.second;
            }

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";
        }

        ss << "]},";
    }

    ss << "]";

    end_chart(w, ss);

    // 2. Display the current currency breakdown

    auto ss2 = start_chart(w, "Current Allocation Breakdown", "pie", "allocation_breakdown_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Classes',";
    ss2 << "colorByPoint: true,";
    ss2 << "data: [";

    for(size_t i = 0; i < names.size(); ++i){
        ss2 << "{ name: '" << names[i] << "',";
        ss2 << "y: ";

        std::map<size_t, budget::money> asset_amounts;

        for (auto& asset_value : sorted_asset_values) {
            auto& asset = get_asset(asset_value.asset_id);

            if(asset.portfolio){
                auto amount = asset_value.amount * exchange_rate(asset.currency);

                if(i == 0 && asset.int_stocks){
                    asset_amounts[asset_value.asset_id] = amount * (float(asset.int_stocks) / 100.0f);
                }

                if(i == 1 && asset.dom_stocks){
                    asset_amounts[asset_value.asset_id] = amount * (float(asset.dom_stocks) / 100.0f);
                }

                if(i == 2 && asset.bonds){
                    asset_amounts[asset_value.asset_id] = amount * (float(asset.bonds) / 100.0f);
                }

                if(i == 3 && asset.cash){
                    asset_amounts[asset_value.asset_id] = amount * (float(asset.cash) / 100.0f);
                }
            }
        }

        budget::money sum;

        for (auto& asset : asset_amounts) {
            sum += asset.second;
        }

        ss2 << budget::to_flat_string(sum);

        ss2 << "},";
    }

    ss2 << "]},";

    ss2 << "]";

    end_chart(w, ss2);

    page_end(w, req, res);
}

void budget::net_worth_currency_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Net Worth Graph")) {
        return;
    }

    std::set<std::string> currencies;

    for (auto& asset : all_user_assets()) {
        currencies.insert(asset.currency);
    }

    budget::html_writer w(content_stream);

    // 1. Display the currency breakdown over time

    auto ss = start_time_chart(w, "Net worth by currency", "area", "currency_time_graph");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(tooltip: {split: true},)=====";
    ss << R"=====(plotOptions: {area: {stacking: 'percent'}},)=====";

    ss << "series: [";

    auto sorted_asset_values = all_sorted_asset_values();

    for (auto& currency : currencies) {
        ss << "{ name: '" << currency << "',";
        ss << "data: [";

        std::map<size_t, budget::money> asset_amounts;

        auto it  = sorted_asset_values.begin();
        auto end = sorted_asset_values.end();

        while (it != end) {
            auto date = it->set_date;

            while (it->set_date == date) {
                if (get_asset(it->asset_id).currency == currency) {
                    asset_amounts[it->asset_id] = it->amount * exchange_rate(get_asset(it->asset_id).currency, date);
                }

                ++it;
            }

            budget::money sum;

            for (auto& asset : asset_amounts) {
                sum += asset.second;
            }

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";
        }

        ss << "]},";
    }

    ss << "]";

    end_chart(w, ss);

    // 2. Display the current currency breakdown

    auto ss2 = start_chart(w, "Current Currency Breakdown", "pie", "currency_breakdown_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Currencies',";
    ss2 << "colorByPoint: true,";
    ss2 << "data: [";

    for (auto& currency : currencies) {
        ss2 << "{ name: '" << currency << "',";
        ss2 << "y: ";

        std::map<size_t, budget::money> asset_amounts;

        for (auto& asset_value : sorted_asset_values) {
            if (get_asset(asset_value.asset_id).currency == currency) {
                asset_amounts[asset_value.asset_id] = asset_value.amount * exchange_rate(get_asset(asset_value.asset_id).currency);
            }
        }

        budget::money sum;

        for (auto& asset : asset_amounts) {
            sum += asset.second;
        }

        ss2 << budget::to_flat_string(sum);

        ss2 << "},";
    }

    ss2 << "]},";

    ss2 << "]";

    end_chart(w, ss2);

    page_end(w, req, res);
}

void budget::portfolio_status_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Portfolio")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_asset_portfolio(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::portfolio_currency_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Portfolio Graph")) {
        return;
    }

    std::set<std::string> currencies;

    for (auto& asset : all_user_assets()) {
        if (asset.portfolio) {
            currencies.insert(asset.currency);
        }
    }

    budget::html_writer w(content_stream);

    // 1. Display the currency breakdown over time

    auto ss = start_time_chart(w, "Portfolio by currency", "area", "portfolio_currency_graph");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Sum' }},)=====";
    ss << R"=====(tooltip: {split: true},)=====";
    ss << R"=====(plotOptions: {area: {stacking: 'percent'}},)=====";

    ss << "series: [";

    auto sorted_asset_values = all_sorted_asset_values();

    for (auto& currency : currencies) {
        ss << "{ name: '" << currency << "',";
        ss << "data: [";

        std::map<size_t, budget::money> asset_amounts;

        auto it  = sorted_asset_values.begin();
        auto end = sorted_asset_values.end();

        while (it != end) {
            auto date = it->set_date;

            while (it->set_date == date) {
                auto& asset = get_asset(it->asset_id);

                if (asset.currency == currency && asset.portfolio) {
                    asset_amounts[it->asset_id] = it->amount * exchange_rate(asset.currency, date);
                }

                ++it;
            }

            budget::money sum;

            for (auto& asset : asset_amounts) {
                sum += asset.second;
            }

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";
        }

        ss << "]},";
    }

    ss << "]";

    end_chart(w, ss);

    // 2. Display the current currency breakdown

    auto ss2 = start_chart(w, "Current Currency Breakdown", "pie", "currency_breakdown_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Currencies',";
    ss2 << "colorByPoint: true,";
    ss2 << "data: [";

    for (auto& currency : currencies) {
        ss2 << "{ name: '" << currency << "',";
        ss2 << "y: ";

        std::map<size_t, budget::money> asset_amounts;

        for (auto& asset_value : sorted_asset_values) {
            auto& asset = get_asset(asset_value.asset_id);

            if (asset.currency == currency && asset.portfolio) {
                asset_amounts[asset_value.asset_id] = asset_value.amount * exchange_rate(asset.currency);
            }
        }

        budget::money sum;

        for (auto& asset : asset_amounts) {
            sum += asset.second;
        }

        ss2 << budget::to_flat_string(sum);

        ss2 << "},";
    }

    ss2 << "]},";

    ss2 << "]";

    end_chart(w, ss2);

    page_end(w, req, res);
}

void budget::portfolio_graph_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Portfolio Graph")) {
        return;
    }

    budget::html_writer w(content_stream);

    auto ss = start_time_chart(w, "Portfolio", "area");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Portfolio' }},)=====";

    ss << R"=====(subtitle: {)=====";
    ss << "text: '" << get_portfolio_value() << " __currency__',";
    ss << R"=====(floating:true, align:"right", verticalAlign: "top", style: { fontWeight: "bold", fontSize: "inherit" })=====";
    ss << R"=====(},)=====";

    ss << "series: [";

    ss << "{ name: 'Portfolio',";
    ss << "data: [";

    std::map<size_t, budget::money> asset_amounts;

    auto sorted_asset_values = all_sorted_asset_values();

    auto it  = sorted_asset_values.begin();
    auto end = sorted_asset_values.end();

    while (it != end) {
        auto date = it->set_date;

        while (it->set_date == date) {
            auto& asset = get_asset(it->asset_id);

            if (asset.portfolio) {
                asset_amounts[it->asset_id] = it->amount * exchange_rate(asset.currency, date);
            }

            ++it;
        }

        budget::money sum;

        for (auto& asset : asset_amounts) {
            sum += asset.second;
        }

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);

    page_end(w, req, res);
}

void budget::rebalance_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Rebalance")) {
        return;
    }

    // 1. Display the rebalance table

    budget::html_writer w(content_stream);
    budget::show_asset_rebalance(w);

    make_tables_sortable(w);

    w << R"=====(<div class="row">)=====";

    // 2. Display the current allocation

    w << R"=====(<div class="col-lg-6 col-md-12">)=====";

    // Collect the amounts per asset

    std::map<size_t, budget::money> asset_amounts;

    for (auto& asset : all_user_assets()) {
        if (asset.portfolio) {
            asset_amounts[asset.id] = get_asset_value(asset);
        }
    }

    // Compute the colors for each asset that will be displayed

    std::map<size_t, size_t> colors;

    for (auto& asset : all_user_assets()) {
        if (asset.portfolio && (asset_amounts[asset.id] || asset.portfolio_alloc)) {
            if (!colors.count(asset.id)) {
                auto c           = colors.size();
                colors[asset.id] = c;
            }
        }
    }

    // Compute the colors for the first graph

    std::stringstream current_ss;

    current_ss << R"=====(var current_base_colors = ["#7cb5ec", "#434348", "#90ed7d", "#f7a35c", "#8085e9", "#f15c80", "#e4d354", "#2b908f", "#f45b5b", "#91e8e1", "red", "blue", "green"];)=====";

    current_ss << "var current_pie_colors = (function () {";
    current_ss << "var colors = [];";

    for (auto& asset_amount : asset_amounts) {
        if (asset_amount.second) {
            current_ss << "colors.push(current_base_colors[" << colors[asset_amount.first] << "]);";
        }
    }

    current_ss << "return colors;";
    current_ss << "}());";

    auto ss = start_chart(w, "Current Allocation", "pie", "current_allocation_graph");

    ss << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss << "series: [";

    ss << "{ name: 'Assets',";
    ss << "colorByPoint: true,";
    ss << "colors: current_pie_colors,";
    ss << "data: [";

    budget::money sum;

    for (auto& asset_amount : asset_amounts) {
        auto amount = asset_amount.second;

        if (amount) {
            auto& asset      = get_asset(asset_amount.first);
            auto conv_amount = amount * exchange_rate(asset.currency);

            ss << "{ name: '" << asset.name << "',";
            ss << "y: ";
            ss << budget::to_flat_string(conv_amount);
            ss << "},";

            sum += conv_amount;
        }
    }

    ss << "]},";

    ss << "]";

    current_ss << ss.str();

    end_chart(w, current_ss);

    w << R"=====(</div>)=====";

    // 3. Display the desired allocation

    // Compute the colors for the second graph

    std::stringstream desired_ss;

    desired_ss << R"=====(var desired_base_colors = ["#7cb5ec", "#434348", "#90ed7d", "#f7a35c", "#8085e9", "#f15c80", "#e4d354", "#2b908f", "#f45b5b", "#91e8e1", "red", "blue", "green"];)=====";

    desired_ss << "var desired_pie_colors = (function () {";
    desired_ss << "var colors = [];";

    for (auto& asset : all_user_assets()) {
        if (asset.portfolio && asset.portfolio_alloc) {
            desired_ss << "colors.push(desired_base_colors[" << colors[asset.id] << "]);";
        }
    }

    desired_ss << "return colors;";
    desired_ss << "}());";

    w << R"=====(<div class="col-lg-6 col-md-12">)=====";

    auto ss2 = start_chart(w, "Desired Allocation", "pie", "desired_allocation_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Assets',";
    ss2 << "colorByPoint: true,";
    ss2 << "colors: desired_pie_colors,";
    ss2 << "data: [";

    for (auto& asset : all_user_assets()) {
        if (asset.portfolio && asset.portfolio_alloc) {
            ss2 << "{ name: '" << asset.name << "',";
            ss2 << "y: ";
            ss2 << budget::to_flat_string(sum * (static_cast<float>(asset.portfolio_alloc) / 100.0f));
            ss2 << "},";
        }
    }

    ss2 << "]},";

    ss2 << "]";

    desired_ss << ss2.str();

    end_chart(w, desired_ss);

    w << R"=====(</div>)=====";

    w << R"=====(</div>)=====";

    page_end(w, req, res);
}

