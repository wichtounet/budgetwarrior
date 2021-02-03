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
#include "http.hpp"
#include "data.hpp"
#include "date.hpp"
#include "money.hpp"
#include "server_lock.hpp"
#include "logging.hpp"

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
budget::server_lock shares_lock;

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
        LOG_F(ERROR, "Price(v3): yfinance_quote.py returned nothing");

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

} // end of anonymous namespace

void budget::load_share_price_cache(){
    std::string file_path = budget::path_to_budget_file("share_price.cache");
    std::ifstream file(file_path);

    if (!file.is_open() || !file.good()){
        LOG_F(INFO, "Impossible to load Share Price Cache");
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

    LOG_F(INFO, "Share Price Cache has been loaded from {}", file_path);
    LOG_F(INFO, "Share Price Cache has {} entries", share_prices.size());
}

void budget::save_share_price_cache() {
    std::string file_path = budget::path_to_budget_file("share_price.cache");
    std::ofstream file(file_path);

    if (!file.is_open() || !file.good()){
        LOG_F(INFO, "Impossible to save Share Price Cache");
        return;
    }

    {
        server_lock_guard l(shares_lock);

        for (auto& [key, value] : share_prices) {
            if (value != budget::money(1)) {
                data_writer writer;
                writer << key.date;
                writer << key.ticker;
                writer << value;
                file << writer.to_string() << std::endl;
            }
        }
    }

    LOG_F(INFO, "Share Price Cache has been saved to {}", file_path);
    LOG_F(INFO, "Share Price Cache has {} entries", share_prices.size());
}

void budget::prefetch_share_price_cache(){
    std::set<std::string> tickers;

    {
        server_lock_guard l(shares_lock);

        // Collect all the tickers
        for (auto& [key, value] : share_prices) {
            tickers.insert(key.ticker);
        }
    }

    // Prefetch the current prices
    for (auto & ticker : tickers) {
        share_price(ticker);
    }

    LOG_F(INFO, "Share Price Cache has been prefetched");
    LOG_F(INFO, "Share Price Cache has {} entries", share_prices.size());
}

budget::money budget::share_price(const std::string& ticker){
    return share_price(ticker, budget::local_day());
}

budget::money budget::share_price(const std::string& ticker, budget::date d){
    auto date = get_valid_date(d);

    share_price_cache_key key(date, ticker);

    {
        server_lock_guard l(shares_lock);

        if (share_prices.count(key)) {
            return share_prices[key];
        }
    }

    // Note: we use a range for two reasons
    // 1) Handle potential holidays, so we have a range in the past
    // 2) Opportunistically grab several quotes in the past and future to save on API calls
    auto start_date = d - budget::days(10);
    auto end_date   = d + budget::days(10);
    auto quotes     = get_share_price_v3(ticker, start_date, end_date);

    server_lock_guard l(shares_lock);

    // If the API did not find anything, it must mean that the ticker is
    // invalid
    if (quotes.empty()) {
        LOG_F(ERROR,
              "Price: Could not find quotes for {} for date {} ({}-{})",
              ticker,
              budget::to_string(d),
              budget::to_string(start_date),
              budget::to_string(end_date));

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

            LOG_F(INFO, "Price: Possible holiday on {}, retrying on {}", budget::to_string(date), budget::to_string(next_date));

            share_price_cache_key next_key(next_date, ticker);
            if (share_prices.count(next_key)) {
                share_prices[key] = share_prices[next_key];
                break;
            }
        }
    }

    if (!share_prices.count(key)) {
        LOG_F(ERROR, "Price: Unable to find data for {} on {}", ticker, budget::to_string(date));
        share_prices[key] = money(1);
        return money(1);
    }

    LOG_F(INFO, "Price: Share price ({}) ticker {} = {}", budget::to_string(date), ticker, budget::to_string(share_prices[key]));

    return share_prices[key];
}
