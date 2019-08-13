//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <tuple>
#include <utility>
#include <iostream>
#include <sstream>

#include "cpp_utils/string.hpp"

#include "share.hpp"
#include "config.hpp"
#include "server.hpp"
#include "http.hpp"
#include "date.hpp"

namespace {

struct share_price_cache_key {
    budget::date date;
    std::string ticker;

    share_price_cache_key(budget::date date, std::string ticker) : date(date), ticker(ticker) {}

    friend bool operator<(const share_price_cache_key & lhs, const share_price_cache_key & rhs){
        return std::tie(lhs.date, lhs.ticker) < std::tie(rhs.date, rhs.ticker);
    }

    friend bool operator==(const share_price_cache_key & lhs, const share_price_cache_key & rhs){
        return std::tie(lhs.date, lhs.ticker) == std::tie(rhs.date, rhs.ticker);
    }
};

std::map<share_price_cache_key, double> share_prices;

budget::date get_valid_date(budget::date d){
    // We cannot get closing price in the future, so we use the day before date
    if (d >= budget::local_day()) {
        auto now = std::chrono::system_clock::now();
        auto tt  = std::chrono::system_clock::to_time_t(now);
        auto tm  = *std::localtime(&tt);

        // We make sure that we are on a new U.S: day
        // TODO This should be done by getting the current time in the U.S.
        if (tm.tm_hour > 15) {
            return get_valid_date(budget::local_day() - budget::days(1));
        } else {
            return get_valid_date(budget::local_day() - budget::days(2));
        }
    }

    auto dow = d.day_of_the_week();

    if (dow == 6 || dow == 7){
        return get_valid_date(d - budget::days(dow - 5));
    }

    return d;
}

// V2 is using alpha_vantage
double get_share_price_v2(const std::string& quote, const budget::date& date, int /*depth*/ = 0) {
    if (!budget::config_contains("alpha_vantage_api_key")) {
        std::cout << "ERROR: Price(v2): Need Alpha Vantage API KEY configured to work" << std::endl;

        return  1.0;
    }

    auto api_key = budget::config_value("alpha_vantage_api_key");

    //https://www.alphavantage.co/query\?function\=TIME_SERIES_DAILY\&symbol\=CHDVD.SWI\&interval\=5min\&apikey\=API_KEY

    httplib::SSLClient cli("www.alphavantage.co", 443);

    auto date_str = budget::date_to_string(date);
    std::string api_complete = "/query?function=TIME_SERIES_DAILY&symbol=" + quote + "&outputsize=full&apikey=" + api_key;

    auto res = cli.Get(api_complete.c_str());

    if (!res) {
        std::cout << "ERROR: Price(v2): No response" << std::endl;
        std::cout << "ERROR: Price(v2): URL is " << api_complete << std::endl;

        return  1.0;
    } else if (res->status != 200) {
        std::cout << "ERROR: Price(v2): Error response " << res->status << std::endl;
        std::cout << "ERROR: Price(v2): URL is " << api_complete << std::endl;
        std::cout << "ERROR: Price(v2): Response is " << res->body << std::endl;

        return  1.0;
    } else {
        std::stringstream bodyreader(res->body);

        std::vector<std::string> lines;

        bool data = false;
        std::string line;
        while(std::getline(bodyreader, line)) {
            cpp::trim(line);
            if (line.size() <= 3) {
                continue;
            }

            if (line.find("Time Series") != std::string::npos) {
                data = true;
            } else if(data) {
                lines.emplace_back(line);
            }
        }

        for (size_t i = 0; i < lines.size(); i += 6){
            std::string date_str(lines[i].begin() + 1, lines[i].begin() + 11);
            std::string value(lines[i+4].begin() + 13, lines[i+4].end() - 2);

            share_price_cache_key key(budget::from_string(date_str), quote);
            share_prices[key] = budget::to_number<float>(value);
        }

        share_price_cache_key key(date, quote);
        if (share_prices.count(key) ){
            return share_prices[key];
        } else {
            for (size_t i = 0; i < 4; ++i){
                auto next_date = get_valid_date(date - budget::days(1));
                std::cout << "INFO: Price(v2): Possible holiday, retrying on previous day" << std::endl;
                std::cout << "INFO: Date was " << date << " retrying with " << next_date << std::endl;

                // Opportunistically check the cache for previous day!
                share_price_cache_key key(next_date, quote);
                if (share_prices.count(key)) {
                    return share_prices[key];
                }
            }
        }

        return 1.0;
    }
}

// V1 is using cloud.iexapis.com
double get_share_price_v1(const std::string& quote, const budget::date& date, int depth = 0) {
    if (!budget::config_contains("iex_cloud_token")) {
        std::cout << "ERROR: Price(v1): Need IEX cloud token configured to work" << std::endl;

        return  1.0;
    }

    auto token = budget::config_value("iex_cloud_token");

    httplib::SSLClient cli("cloud.iexapis.com", 443);

    auto date_str = budget::date_to_string(date);
    std::string api_complete = "/beta/stock/" + quote + "/chart/date/" + date_str + "?chartByDay=true&token=" + token;

    auto res = cli.Get(api_complete.c_str());

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
        // Pseudo handling of holidays
        if (res->body == "[]") {
            if (depth <= 3) {
                auto next_date = get_valid_date(date - budget::days(1));
                std::cout << "INFO: Price(v1): Possible holiday, retrying on previous day" << std::endl;
                std::cout << "INFO: Date was " << date << " retrying with " << next_date << std::endl;

                // Opportunistically check the cache for previous day!
                share_price_cache_key key(next_date, quote);
                if (share_prices.count(key)) {
                    return share_prices[key];
                } else {
                    return get_share_price_v1(quote, next_date, depth + 1);
                }
            }
        }

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

void budget::load_share_price_cache(){
    std::string file_path = budget::path_to_budget_file("share_price.cache");
    std::ifstream file(file_path);

    if (!file.is_open() || !file.good()){
        std::cout << "INFO: Impossible to load Share Price Cache" << std::endl;
        return;
    }

    std::string line;
    while (file.good() && getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        auto parts = split(line, ':');

        share_price_cache_key key(budget::from_string(parts[0]), parts[1]);
        share_prices[key] = budget::to_number<double>(parts[2]);
    }

    if (budget::is_server_running()) {
        std::cout << "INFO: Share Price Cache has been loaded from " << file_path << std::endl;
        std::cout << "INFO: Share Price Cache has " << share_prices.size() << " entries " << std::endl;
    }
}

void budget::save_share_price_cache() {
    std::string file_path = budget::path_to_budget_file("share_price.cache");
    std::ofstream file(file_path);

    if (!file.is_open() || !file.good()){
        std::cout << "INFO: Impossible to save Share Price Cache" << std::endl;
        return;
    }

    for (auto & pair : share_prices) {
        if (pair.second != 1.0) {
            auto& key = pair.first;

            file << budget::date_to_string(key.date) << ':' << key.ticker << ':' << pair.second << std::endl;
        }
    }

    if (budget::is_server_running()) {
        std::cout << "INFO: Share Price Cache has been saved to " << file_path << std::endl;
        std::cout << "INFO: Share Price Cache has " << share_prices.size() << " entries " << std::endl;
    }
}

void budget::refresh_share_price_cache(){
    // Refresh the prices for each value
    for (auto& pair : share_prices) {
        auto& key = pair.first;

        share_prices[key] = get_share_price_v1(key.ticker, key.date);
    }

    // Prefetch the current prices
    for (auto & pair : share_prices) {
        auto& key = pair.first;

        share_price(key.ticker);
    }

    if (budget::is_server_running()) {
        std::cout << "INFO: Share Price Cache has been refreshed" << std::endl;
        std::cout << "INFO: Share Price Cache has " << share_prices.size() << " entries " << std::endl;
    }
}

double budget::share_price(const std::string& ticker){
    return share_price(ticker, budget::local_day());
}

double budget::share_price(const std::string& ticker, budget::date d){
    auto date = get_valid_date(d);

    share_price_cache_key key(date, ticker);

    if (!share_prices.count(key)) {
        auto price = get_share_price_v1(ticker, date);

        if (budget::is_server_running()) {
            std::cout << "INFO: Share: Price (" << date << ")"
                      << " ticker " << ticker << " = " << price << std::endl;
        }

        share_prices[key] = price;
    }

    return share_prices[key];
}
