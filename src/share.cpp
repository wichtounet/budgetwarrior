//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <tuple>
#include <utility>
#include <iostream>

#include "share.hpp"
#include "config.hpp"
#include "server.hpp"
#include "http.hpp"
#include "date.hpp"

namespace {

std::map<std::tuple<std::string, std::string>, double> share_prices;

// V1 is using cloud.iexapis.com
double get_share_price_v1(const std::string& quote, const std::string& date) {
    std::string token;
    if (!budget::config_contains("iex_cloud_token")) {
        std::cout << "ERROR: Price(v1): Need IEX cloud token configured to work" << std::endl;

        return  1.0;
    }

    httplib::SSLClient cli("cloud.iexapis.com", 443);

    std::string api_complete = "/beta/stock/" + quote + "/chart/date/" + date + "?chartByDay=true&token=" + token;

    auto res = cli.get(api_complete.c_str());

    if (!res) {
        std::cout << "ERROR: Price(v1): No response" << std::endl;
        std::cout << "ERROR: Price(v1): URL is " << api_complete << std::endl;

        return  1.0;
    } else if (res->status != 200) {
        std::cout << "ERROR: Price(v1): Error response " << res->status << std::endl;
        std::cout << "ERROR: Price(v1): URL is " << api_complete << std::endl;
        std::cout << "ERROR: Price(v1): Response is " << res->body << std::endl;

        return  1.0;
    } else {
        // Example
        // [{"date":"2019-03-01","open":174.28,"close":174.97,"high":175.15,"low":172.89,"volume":25886167,"uOpen":174.28,"uClose":174.97,"uHigh":175.15,"uLow":172.89,"uVolume":25886167,"change":1.82,"changePercent":1.0511,"label":"Mar
        // 1, 19","changeOverTime":172.237624}]
        auto& buffer      = res->body;
        std::string start = "\"close\":";
        std::string stop  = ",\"high\":";

        if (buffer.find(start) == std::string::npos || buffer.find(stop) == std::string::npos) {
            std::cout << "ERROR: Price(v1): Error parsing share prices" << std::endl;
            std::cout << "ERROR: Price(v1): URL is " << api_complete << std::endl;
            std::cout << "ERROR: Price(v1): Response is " << res->body << std::endl;

            return  1.0;
        } else {
            std::string ratio_result(buffer.begin() + buffer.find(start) + start.size(), buffer.begin() + buffer.find(stop));

            return atof(ratio_result.c_str());
        }
    }
}

} // end of anonymous namespace

void budget::refresh_share_price_cache(){
    // Refresh the prices for each value
    for (auto& pair : share_prices) {
        auto& key  = pair.first;
        auto date  = std::get<0>(key);
        auto quote = std::get<1>(key);

        share_prices[key] = get_share_price_v1(quote, date);
    }

    // Prefetch the current prices
    for (auto & pair : share_prices) {
        auto& key  = pair.first;
        auto quote = std::get<1>(key);

        share_price(quote);
    }

    std::cout << "INFO: Share Price Cache has been refreshed" << std::endl;
    std::cout << "INFO: Share Price Cache has " << share_prices.size() << " entries " << std::endl;
}

double budget::share_price(const std::string& quote){
    return share_price(quote, budget::local_day());
}

double budget::share_price(const std::string& quote, budget::date d){
    auto date_str = budget::date_to_string(d);
    auto key      = std::make_tuple(date_str, quote);

    if (!share_prices.count(key)) {
        auto price = get_share_price_v1(quote, date_str);

        if (budget::is_server_running()) {
            std::cout << "INFO: Share: Price (" << date_str << ")"
                      << " quote " << quote << " = " << price << std::endl;
        }

        share_prices[key] = price;
    }

    return share_prices[key];
}
