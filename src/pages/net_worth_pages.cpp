//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "cpp_utils/assert.hpp"

#include "assets.hpp"
#include "liabilities.hpp"

#include "writer.hpp"
#include "pages/net_worth_pages.hpp"
#include "http.hpp"
#include "currency.hpp"
#include "config.hpp"
#include "share.hpp"

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

    // If one asset has no group, we disable grouping
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

void budget::asset_graph_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Asset Graph")) {
        return;
    }

    budget::html_writer w(content_stream);

    auto& asset = req.matches.size() == 2
        ? get_asset(to_number<size_t>(req.matches[1]))
        : *all_user_assets().begin();

    if (req.matches.size() == 2) {
        w << title_begin << "Asset Graph" << budget::asset_selector{"assets/graph", to_number<size_t>(req.matches[1])} << title_end;
    } else {
        w << title_begin << "Asset Graph" << budget::asset_selector{"assets/graph", 0} << title_end;
    }

    asset_graph(w, "", asset);

    // Display additional information for share-based assets
    if (asset.share_based) {
        int64_t bought_shares = 0;
        int64_t sold_shares  = 0;

        budget::money average_buy_price;
        budget::money average_sell_price;

        auto current_price = budget::money(1) * share_price(asset.ticker);
        date first_date    = local_day();
        bool first_date_set = false;

        for (auto& share : all_asset_shares()) {
            if (share.asset_id == asset.id) {
                if (share.is_buy()) {
                    bought_shares += share.shares;
                    average_buy_price += (float)share.shares * share.price;
                }

                if (share.is_sell()) {
                    sold_shares += -share.shares;
                    average_sell_price += (float)-share.shares * share.price;
                }

                if (!first_date_set) {
                    first_date     = share.date;
                    first_date_set = true;
                }
            }
        }

        average_buy_price /= bought_shares;

        auto owned_shares = bought_shares - sold_shares;

        w << p_begin << "Number of shares: " << owned_shares << p_end;
        w << p_begin << "Average price: " << average_buy_price << p_end;
        w << p_begin << "Current price: " << current_price << p_end;
        w << p_begin << "Invested: " << (float) owned_shares * average_buy_price << p_end;
        w << p_begin << "Value: " << (float) owned_shares * current_price << p_end;
        w << p_begin << "Current profit: " << (float) owned_shares * (current_price - average_buy_price) << p_end;
        w << p_begin << "ROI: " << (100.0f / (average_buy_price / current_price)) - 100.0f << "%" << p_end;
        w << p_begin << "First Invest: " << budget::to_string(first_date) << p_end;

        // TODO This is not entirely correct, since this should use
        // the date of sold and buy to have the correct profit
        if (sold_shares) {
            average_sell_price /= sold_shares;

            w << p_begin << p_end;
            w << p_begin << "Sold shares: " << sold_shares << p_end;
            w << p_begin << "Average sold price: " << average_sell_price << p_end;
            w << p_begin << "Realized profit: " << (float) sold_shares * (average_sell_price - average_buy_price) << p_end;
            w << p_begin << "Realized ROI: " << (100.0f / (average_buy_price / average_sell_price)) - 100.0f << "%" << p_end;
        }
    }

    page_end(w, req, res);
}

void budget::asset_graph(budget::html_writer& w, const std::string style, asset& asset) {
   auto ss = start_time_chart(w, asset.name + "(" + asset.currency + ")", "area", "asset_graph", style);

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << R"=====(subtitle: {)=====";
    ss << "text: '" << get_asset_value(asset) << " " << asset.currency << "',";
    ss << R"=====(floating:true, align:"right", verticalAlign: "top", style: { fontWeight: "bold", fontSize: "inherit" })=====";
    ss << R"=====(},)=====";

    ss << "series: [";

    ss << "{ name: 'Value',";
    ss << "data: [";

    auto date     = budget::asset_start_date(asset);
    auto end_date = budget::local_day();

    while (date <= end_date) {
        auto sum = get_asset_value(asset, date);

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";

        date += days(1);
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);
}

void budget::net_worth_graph(budget::html_writer& w, const std::string style, bool card) {
    // if the user does not use assets, this graph does not make sense
    if (all_assets().empty() || all_asset_values().empty()) {
        return;
    }

    auto current_net_worth = get_net_worth();
    auto now               = budget::local_day();
    auto y_net_worth       = get_net_worth({now.year(), 1, 1});
    auto m_net_worth       = get_net_worth(now - days(now.day() - 1));
    auto ytd_growth        = 100.0 * ((1 / (y_net_worth / current_net_worth)) - 1);
    auto mtd_growth        = 100.0 * ((1 / (m_net_worth / current_net_worth)) - 1);

    if (card) {
        w << R"=====(<div class="card">)=====";

        w << R"=====(<div class="card-header card-header-primary">)=====";
        w << R"=====(<div class="float-left">Net Worth</div>)=====";
        w << R"=====(<div class="float-right">)=====";
        w << current_net_worth << " __currency__ (YTD: " << ytd_growth << "% MTD: " << mtd_growth << "%)";
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
        ss << "text: '" << current_net_worth << " __currency__ (YTD: " << ytd_growth << "% MTD: " << mtd_growth << "%)',";
        ss << R"=====(floating:true, align:"right", verticalAlign: "top", style: { fontWeight: "bold", fontSize: "inherit" })=====";
        ss << R"=====(},)=====";
    }

    ss << "series: [";

    ss << "{ name: 'Net Worth',";
    ss << "data: [";

    auto date     = budget::asset_start_date();
    auto end_date = budget::local_day();

    while (date <= end_date) {
        budget::money sum;

        for (auto & asset : all_user_assets()) {
            sum += get_asset_value_conv(asset, date);
        }

        for (auto & asset : all_liabilities()) {
            sum -= get_liability_value_conv(asset, date);
        }

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";

        date += days(1);
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);

    if (card) {
        w << R"=====(</div>)====="; //card-body
        w << R"=====(</div>)====="; //card
    }

    if (!card) {
        w << p_begin << "MTD Change " << current_net_worth - m_net_worth << " __currency__" << p_end;
        w << p_begin << "MTD Growth " << mtd_growth << " %" << p_end;

        w << p_begin << "YTD Change " << current_net_worth - y_net_worth << " __currency__" << p_end;
        w << p_begin << "YTD Growth " << ytd_growth << " %" << p_end;
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

    budget::html_writer w(content_stream);

    // 1. Display the currency breakdown over time

    auto ss = start_time_chart(w, "Net worth allocation", "area", "allocation_time_graph");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(tooltip: {split: true},)=====";
    ss << R"=====(plotOptions: {area: {stacking: 'percent'}},)=====";

    ss << "series: [";

    for (auto & clas : all_asset_classes()) {
        ss << "{ name: '" << clas.name << "',";
        ss << "data: [";

        auto date     = budget::asset_start_date();
        auto end_date = budget::local_day();

        while (date <= end_date) {
            budget::money sum;

            for (auto & asset : all_user_assets()) {
                sum += get_asset_value_conv(asset, date) * (float(get_asset_class_allocation(asset, clas)) / 100.0f);
            }

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";

            date += days(1);
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

    for (auto & clas : all_asset_classes()) {
        ss2 << "{ name: '" << clas.name << "',";
        ss2 << "y: ";

        budget::money sum;

        for (auto & asset : all_user_assets()){
            sum += get_asset_value_conv(asset) * (float(get_asset_class_allocation(asset, clas)) / 100.0f);
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

    budget::html_writer w(content_stream);

    // 1. Display the currency breakdown over time

    auto ss = start_time_chart(w, "Portfolio allocation", "area", "allocation_time_graph");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(tooltip: {split: true},)=====";
    ss << R"=====(plotOptions: {area: {stacking: 'percent'}},)=====";

    ss << "series: [";

    for (auto & clas : all_asset_classes()) {
        ss << "{ name: '" << clas.name << "',";
        ss << "data: [";

        auto date     = budget::asset_start_date();
        auto end_date = budget::local_day();

        while (date <= end_date) {
            budget::money sum;

            for (auto & asset : all_user_assets()) {
                if (asset.portfolio) {
                    sum += get_asset_value_conv(asset, date) * (float(get_asset_class_allocation(asset, clas)) / 100.0f);
                }
            }

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";

            date += days(1);
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

    for (auto & clas : all_asset_classes()) {
        ss2 << "{ name: '" << clas.name << "',";
        ss2 << "y: ";

        budget::money sum;

        for (auto & asset : all_user_assets()) {
            if (asset.portfolio) {
                sum += get_asset_value_conv(asset) * (float(get_asset_class_allocation(asset, clas)) / 100.0f);
            }
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

    for (auto& currency : currencies) {
        ss << "{ name: '" << currency << "',";
        ss << "data: [";

        auto date     = budget::asset_start_date();
        auto end_date = budget::local_day();

        while (date <= end_date) {
            budget::money sum;

            for (auto & asset : all_user_assets()) {
                if (asset.currency == currency) {
                    sum += get_asset_value_conv(asset, date);
                }
            }

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";

            date += days(1);
        }

        ss << "]},";
    }

    ss << "]";

    end_chart(w, ss);

    // 2. Display the value in each currency

    for (auto& currency : currencies) {
        budget::money net_worth;

        for (auto & asset : all_user_assets()) {
            net_worth += get_asset_value_conv(asset, currency);
        }

        w << p_begin << "Net worth in " << currency << " : " << net_worth << " " << currency << p_end;
    }

    // 3. Display the current currency breakdown

    auto ss2 = start_chart(w, "Current Currency Breakdown", "pie", "currency_breakdown_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Currencies',";
    ss2 << "colorByPoint: true,";
    ss2 << "data: [";

    for (auto& currency : currencies) {
        ss2 << "{ name: '" << currency << "',";
        ss2 << "y: ";

        budget::money sum;

        for (auto & asset : all_user_assets()) {
            if (asset.currency == currency) {
                sum += get_asset_value_conv(asset);
            }
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

    for (auto& currency : currencies) {
        ss << "{ name: '" << currency << "',";
        ss << "data: [";

        auto date     = budget::asset_start_date();
        auto end_date = budget::local_day();

        while (date <= end_date) {
            budget::money sum;

            for (auto & asset : all_user_assets()) {
                if (asset.portfolio && asset.currency == currency) {
                    sum += get_asset_value_conv(asset, date);
                }
            }

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";

            date += days(1);
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

        budget::money sum;

        for (auto & asset : all_user_assets()) {
            if (asset.portfolio && asset.currency == currency) {
                sum += get_asset_value_conv(asset);
            }
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

    auto date     = budget::asset_start_date();
    auto end_date = budget::local_day();

    while (date <= end_date) {
        budget::money sum;

        for (auto & asset : all_user_assets()) {
            if (asset.portfolio) {
                sum += get_asset_value_conv(asset, date);
            }
        }

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::to_flat_string(sum) << "],";

        date += days(1);
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);

    page_end(w, req, res);
}

void rebalance_page_base(const httplib::Request& req, httplib::Response& res, bool nocash) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Rebalance")) {
        return;
    }

    // 1. Display the rebalance table

    budget::html_writer w(content_stream);
    budget::show_asset_rebalance(w, nocash);

    make_tables_sortable(w);

    w << R"=====(<div class="row">)=====";

    // 2. Display the current allocation

    w << R"=====(<div class="col-lg-6 col-md-12">)=====";

    // Collect the amounts per asset

    std::map<size_t, budget::money> asset_amounts;

    for (auto& asset : all_user_assets()) {
        if (asset.portfolio) {
            if (nocash && asset.is_cash()) {
                continue;
            }
            asset_amounts[asset.id] = get_asset_value(asset);
        }
    }

    // Compute the colors for each asset that will be displayed

    std::map<size_t, size_t> colors;

    for (auto& asset : all_user_assets()) {
        if (nocash && asset.is_cash()) {
            continue;
        }

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

    for (auto& [asset_id, amount] : asset_amounts) {
        if (amount) {
            current_ss << "colors.push(current_base_colors[" << colors[asset_id] << "]);";
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

    for (auto& [asset_id, amount] : asset_amounts) {
        if (amount) {
            auto& asset      = get_asset(asset_id);
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

void budget::rebalance_page(const httplib::Request& req, httplib::Response& res) {
    rebalance_page_base(req, res, false);
}

void budget::rebalance_nocash_page(const httplib::Request& req, httplib::Response& res) {
    rebalance_page_base(req, res, true);
}
