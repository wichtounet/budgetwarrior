//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "share.hpp"

#include <math.h>

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <tuple>
#include <utility>

#include "assets.hpp"
#include "config.hpp"
#include "cpp_utils/string.hpp"
#include "data.hpp"
#include "data_cache.hpp"
#include "date.hpp"
#include "http.hpp"
#include "logging.hpp"
#include "money.hpp"
#include "server_lock.hpp"

namespace {

struct share_price_cache_key {
    budget::date date;
    std::string ticker;

    share_price_cache_key(budget::date date, std::string ticker) : date(date), ticker(std::move(std::move(ticker))) {}

    friend auto operator<=>(const share_price_cache_key & lhs, const share_price_cache_key & rhs) = default;
};

// We use a struct so that we can store values of 1 that can indicate either
// a valid value or an invalid one
// Without that, we could not store values of 1 in the cache file
struct share_cache_value {
    budget::money value;
    bool valid{};
};

std::map<share_price_cache_key, share_cache_value, std::less<>> share_prices;
budget::server_lock shares_lock;

budget::date get_valid_date(const budget::date & d){
    // We cannot get closing price in the future, so we use the day before date
    if (d >= budget::local_day()) {
        auto now = std::chrono::system_clock::now();
        auto tt  = std::chrono::system_clock::to_time_t(now);

        // We make sure that we are on a new U.S: day
        // TODO This should be done by getting the current time in the U.S.
        if (auto tm  = *std::localtime(&tt); tm.tm_hour > 15) {
            return get_valid_date(budget::local_day() - budget::days(1));
        }

        return get_valid_date(budget::local_day() - budget::days(2));
    }

    if (auto dow = d.day_of_the_week(); dow == 6 || dow == 7){
        return get_valid_date(d - budget::days(dow - 5));
    }

    return d;
}

// When we do not find a value for a ticker, we need to find an invalid value
// In the worst case, we use a value of 1, but sometimes we can do better
// We can try to find the closest date in the past for this ticker
// This function must be called with a lock!
share_cache_value get_invalid_value(const share_price_cache_key & key) {
    auto next_key = key;
    for (size_t i = 0; i < 5; ++i) {
        next_key.date = next_key.date - budget::days(1);

        if (share_prices.contains(next_key)) {
            LOG_F(INFO,
                  "Price: Using invalid previous share price ({}->{}) for {} = {}",
                  budget::to_string(key.date),
                  budget::to_string(next_key.date),
                  key.ticker,
                  budget::to_string(share_prices[next_key].value));
            return {share_prices[next_key].value, false};
        }
    }

    LOG_F(INFO, "Price: Using invalid fixed share price ({}) for {} = 1", budget::to_string(key.date), key.ticker);

    return {budget::money(1), false};
}

// V3 is using Yahoo Finance
// Starting from this version, the get_share_price function must be thread
// safe. This means, it cannot touch the cache itself
std::map<share_price_cache_key, budget::money, std::less<>> get_share_price_v3(const std::string & ticker, budget::date start_date, budget::date end_date) {
    std::string const command =
        "yfinance_quote.py " + ticker + " " + date_to_string(start_date) + " " + date_to_string(end_date);

    auto result = budget::exec_command(command);

    if (result.empty()) {
        LOG_F(ERROR, "Price(v3): yfinance_quote.py returned nothing");

        return {};
    }

    std::map<share_price_cache_key, budget::money, std::less<>> quotes;

    std::stringstream ss(result);

    try {
        std::string line;
        while (getline(ss, line)) {
            budget::data_reader reader;
            reader.parse(line);

            budget::date d{};
            budget::money m;

            reader >> d;
            reader >> m;

            const share_price_cache_key key(d, ticker);
            quotes[key] = m;
        }
    } catch (const budget::date_exception&) {
        return {};
    } catch (const budget::budget_exception&) {
        return {};
    }

    return quotes;
}

} // end of anonymous namespace

void budget::load_share_price_cache(){
    const auto file_path = budget::path_to_budget_file("share_price.cache");

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

        budget::date day{};
        std::string  ticker;
        double value = NAN;

        data_reader reader;
        reader.parse(line);

        reader >> day;
        reader >> ticker;
        reader >> value;

        const share_price_cache_key key(day, ticker);
        share_prices[key] = {budget::money::from_double(value), true};
    }

    LOG_F(INFO, "Share Price Cache has been loaded from {}", file_path.string());
    LOG_F(INFO, "Share Price Cache has {} entries", share_prices.size());
}

void budget::save_share_price_cache() {
    const auto file_path = budget::path_to_budget_file("share_price.cache");

    std::ofstream file(file_path);

    if (!file.is_open() || !file.good()){
        LOG_F(INFO, "Impossible to save Share Price Cache");
        return;
    }

    {
        const server_lock_guard l(shares_lock);

        for (auto& [key, value] : share_prices) {
            if (value.valid) {
                data_writer writer;
                writer << key.date;
                writer << key.ticker;
                writer << value.value;
                file << writer.to_string() << std::endl;
            }
        }
    }

    LOG_F(INFO, "Share Price Cache has been saved to {}", file_path.string());
    LOG_F(INFO, "Share Price Cache has {} entries", share_prices.size());
}

void budget::prefetch_share_price_cache(){
    std::set<std::string, std::less<>> tickers;

    {
        const server_lock_guard l(shares_lock);

        // Collect all the tickers
        for (const auto& [key, value] : share_prices) {
            tickers.insert(key.ticker);
        }
    }

    data_cache cache;

    // Prefetch the current prices
    for (const auto & ticker : tickers) {
        if (is_ticker_active(cache, ticker)) {
            share_price(ticker);
        }
    }

    LOG_F(INFO, "Share Price Cache has been prefetched");
    LOG_F(INFO, "Share Price Cache has {} entries", share_prices.size());
}

budget::money budget::share_price(const std::string& ticker){
    return share_price(ticker, budget::local_day());
}

budget::money budget::share_price(const std::string& ticker, budget::date d){
    auto date = get_valid_date(d);

    const share_price_cache_key key(date, ticker);

    // The first step is to get the data from the cache

    {
        const server_lock_guard l(shares_lock);

        if (share_prices.contains(key)) {
            return share_prices[key].value;
        }
    }

    // Note: we use a range for two reasons
    // 1) Handle potential holidays, so we have a range in the past
    // 2) Opportunistically grab several quotes in the past and future to save on API calls
    auto start_date = d - budget::days(10);
    auto end_date   = d + budget::days(10);
    auto quotes     = get_share_price_v3(ticker, start_date, end_date);

    const server_lock_guard l(shares_lock);

    // If the API did not find anything, it must mean that the ticker is
    // invalid
    if (quotes.empty()) {
        LOG_F(ERROR,
              "Price: Could not find quotes for {} for date {} ({}-{})",
              ticker,
              budget::to_string(d),
              budget::to_string(start_date),
              budget::to_string(end_date));

        share_prices[key] = get_invalid_value(key);
        return share_prices[key].value;
    }

    for (const auto & [new_key, new_value] : quotes) {
        share_prices[new_key] = {new_value, true};
    }

    // If it has not been found, it may be a holiday, so we try to get
    // back in time to find a proper value
    if (!share_prices.contains(key)) {
        for (size_t i = 0; i < 4; ++i){
            auto next_date = get_valid_date(date - budget::days(1));

            LOG_F(INFO, "Price: Possible holiday on {}, retrying on {}", budget::to_string(date), budget::to_string(next_date));

            const share_price_cache_key next_key(next_date, ticker);
            if (share_prices.contains(next_key)) {
                share_prices[key] = share_prices[next_key];
                break;
            }
        }
    }

    if (!share_prices.contains(key)) {
        LOG_F(ERROR, "Price: Unable to find data for {} on {}", ticker, budget::to_string(date));
        share_prices[key] = get_invalid_value(key);
        return share_prices[key].value;
    }

    LOG_F(INFO, "Price: Share price ({}) ticker {} = {}", budget::to_string(date), ticker, budget::to_string(share_prices[key].value));

    return share_prices[key].value;
}
