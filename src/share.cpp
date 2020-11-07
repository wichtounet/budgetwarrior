//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <tuple>
#include <utility>
#include <iostream>
#include <sstream>
#include <map>
#include <set>

#include "cpp_utils/string.hpp"

#include "share.hpp"
#include "config.hpp"
#include "server.hpp"
#include "http.hpp"
#include "data.hpp"
#include "date.hpp"
#include "money.hpp"

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

std::map<share_price_cache_key, budget::money> share_prices;

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

std::string exec_command(const std::string& command) {
    std::stringstream output;

    char buffer[1024];

    FILE* stream = popen(command.c_str(), "r");

    while (fgets(buffer, 1024, stream) != NULL) {
        output << buffer;
    }

    pclose(stream);

    return output.str();
}

// V3 is using Yahoo Finance
// Starting from this version, the get_share_price function must be thread
// safe. This means, it cannot touch the cache itself
std::map<share_price_cache_key, budget::money> get_share_price_v3(const std::string & ticker, budget::date start_date, budget::date end_date) {
    std::string command = "yfinance_quote.py " + ticker + " " + date_to_string(start_date) + " " + date_to_string(end_date);

    auto result = exec_command(command);

    if (result.empty()) {
        std::cout << "ERROR: Price(v3): yfinance_quote.py returned nothing" << std::endl;

        return {};
    }

    std::map<share_price_cache_key, budget::money> quotes;

    std::stringstream ss(result);

    try {
        std::string line;
        while (getline(ss, line)) {
            budget::data_reader reader;
            reader.parse(line);

            budget::date  d;
            budget::money m;

            reader >> d;
            reader >> m;

            share_price_cache_key key(d, ticker);
            quotes[key] = m;
        }
    } catch (const budget::date_exception& e) {
        return {};
    } catch (const budget::budget_exception& e) {
        return {};
    }

    return quotes;
}

// V2 is using alpha_vantage
budget::money get_share_price_v2(const std::string& quote, const budget::date& date, int /*depth*/ = 0) {
    if (!budget::config_contains("alpha_vantage_api_key")) {
        std::cout << "ERROR: Price(v2): Need Alpha Vantage API KEY configured to work" << std::endl;

        return budget::money{1};
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

        return budget::money{1};
    } else if (res->status != 200) {
        std::cout << "ERROR: Price(v2): Error response " << res->status << std::endl;
        std::cout << "ERROR: Price(v2): URL is " << api_complete << std::endl;
        std::cout << "ERROR: Price(v2): Response is " << res->body << std::endl;

        return budget::money{1};
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

            share_price_cache_key key(budget::date_from_string(date_str), quote);
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

        return budget::money{1};
    }
}

// V1 is using cloud.iexapis.com
budget::money get_share_price_v1(const std::string& quote, const budget::date& date, int depth = 0) {
    if (!budget::config_contains("iex_cloud_token")) {
        std::cout << "ERROR: Price(v1): Need IEX cloud token configured to work" << std::endl;

        return budget::money{1};
    }

    auto token = budget::config_value("iex_cloud_token");

    httplib::SSLClient cli("cloud.iexapis.com", 443);

    auto date_str = budget::date_to_string(date);
    std::string api_complete = "/beta/stock/" + quote + "/chart/date/" + date_str + "?chartByDay=true&token=" + token;

    auto res = cli.Get(api_complete.c_str());

    if (!res) {
        std::cout << "ERROR: Price(v1): No response" << std::endl;
        std::cout << "ERROR: Price(v1): URL is " << api_complete << std::endl;

        return budget::money{1};
    } else if (res->status != 200) {
        std::cout << "ERROR: Price(v1): Error response " << res->status << std::endl;
        std::cout << "ERROR: Price(v1): URL is " << api_complete << std::endl;
        std::cout << "ERROR: Price(v1): Response is " << res->body << std::endl;

        return budget::money{1};
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

            return budget::money{1};
        } else {
            std::string ratio_result(buffer.begin() + buffer.find(start) + start.size(), buffer.begin() + buffer.find(stop));

            return budget::money::from_double(atof(ratio_result.c_str()));
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

        // Note: For now, we still need to use double instead of money since
        // previous data was written in an invalid format related to money
        // However, this will be able to go away since they are written down
        // with budget::money now

        budget::date day;
        std::string  ticker;
        double       value;

        data_reader reader;
        reader.parse(line);

        reader >> day;
        reader >> ticker;
        reader >> value;

        share_price_cache_key key(day, ticker);
        share_prices[key] = budget::money::from_double(value);
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

    for (auto & [key, value] : share_prices) {
        if (value != budget::money(1)) {
            file << budget::date_to_string(key.date) << ':' << key.ticker << ':' << value << std::endl;
        }
    }

    if (budget::is_server_running()) {
        std::cout << "INFO: Share Price Cache has been saved to " << file_path << std::endl;
        std::cout << "INFO: Share Price Cache has " << share_prices.size() << " entries " << std::endl;
    }
}

void budget::prefetch_share_price_cache(){
    std::set<std::string> tickers;

    // Collect all the tickers
    for (auto & [key, value] : share_prices) {
        tickers.insert(key.ticker);
    }

    // Prefetch the current prices
    for (auto & ticker : tickers) {
        share_price(ticker);
    }

    if (budget::is_server_running()) {
        std::cout << "INFO: Share Price Cache has been prefetched" << std::endl;
        std::cout << "INFO: Share Price Cache has " << share_prices.size() << " entries " << std::endl;
    }
}

budget::money budget::share_price(const std::string& ticker){
    return share_price(ticker, budget::local_day());
}

budget::money budget::share_price(const std::string& ticker, budget::date d){
    auto date = get_valid_date(d);

    share_price_cache_key key(date, ticker);

    if (!share_prices.count(key)) {
        // Note: We use a range in order to handle potential holidays
        // where the stock market is closed
        auto quotes = get_share_price_v3(ticker, d - budget::days(8), d);

        // If the API did not find anything, it must mean that the ticker is
        // invalid
        if (quotes.empty()) {
            if (budget::is_server_running()) {
                std::cout << "INFO: Price: Could not find quotes for " << ticker << std::endl;
            }

            share_prices[key] = money(1);
            return money(1);
        }

        for (auto [new_key, new_value] : quotes) {
            share_prices[new_key] = new_value;
        }

        // If it has not been found, it may be a holiday, so we try to get
        // back in time to find a proper value
        if (!share_prices.count(key)) {
            for (size_t i = 0; i < 4; ++i){
                auto next_date = get_valid_date(date - budget::days(1));

                if (budget::is_server_running()) {
                    std::cout << "INFO: Price: Possible holiday, retrying on previous day" << std::endl;
                    std::cout << "INFO: Date was " << date << " retrying with " << next_date << std::endl;
                }

                share_price_cache_key next_key(next_date, ticker);
                if (share_prices.count(next_key)) {
                    share_prices[key] = share_prices[next_key];
                    break;
                }
            }
        }

        cpp_assert(share_prices.count(key), "Invalid state in share_price");

        if (budget::is_server_running()) {
            std::cout << "INFO: Share: Price (" << date << ")"
                      << " ticker " << ticker << " = " << share_prices[key] << std::endl;
        }
    }

    return share_prices[key];
}
