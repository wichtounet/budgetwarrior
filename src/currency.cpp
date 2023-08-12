//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <tuple>
#include <utility>
#include <iostream>
#include <unordered_map>
#include <format>

#include "currency.hpp"
#include "assets.hpp" // For get_default_currency
#include "http.hpp"
#include "date.hpp"
#include "config.hpp"
#include "logging.hpp"
#include "server_lock.hpp"

namespace {

struct currency_cache_key {
    budget::date date;
    std::string from;
    std::string to;

    currency_cache_key(const budget::date& date, std::string_view from, std::string_view to)
        : date(date), from(from), to(to) {}

    friend bool operator<=>(const currency_cache_key & lhs, const currency_cache_key & rhs) = default;
};

// We use a struct so that we can store values of 1 that can indicate either 
// a valid value or an invalid one
// Without that, we could not store values of 1 in the cache file
struct currency_cache_value {
    double value;
    bool   valid;
};

} // end of anonymous namespace

namespace std {

template <>
struct hash<currency_cache_key> {
    std::size_t operator()(const currency_cache_key & key) const noexcept {
        auto seed = std::hash<budget::date>()(key.date);
        seed ^= std::hash<std::string>()(key.from) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= std::hash<std::string>()(key.to) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        return seed;
    }
};

} // end of namespace std

namespace {

// OPTIM: If necessary, this could be made faster with a two layer cache
// We can take advantages from the low number of currency pair in the first layer
// and then the quick hash from the high number of dates

std::unordered_map<currency_cache_key, currency_cache_value> exchanges;
budget::server_lock exchanges_lock;

// V2 is using api.exchangeratesapi.io
currency_cache_value get_rate_v2(const std::string& from, const std::string& to, const std::string& date = "latest") {
    auto access_key = budget::user_config_value("exchangeratesapi_key", "");

    if (access_key.empty()) {
        return {1.0, false};
    }

    httplib::SSLClient cli("api.exchangeratesapi.io", 443);

    auto url = std::format("/{}?symbols={}&base={}&access_key={}", date, to, from, access_key);

    auto res = cli.Get(url.c_str());

    if (!res) {
        LOG_F(ERROR, "Currency(v2): No response, setting exchange between {} from {} to to 1/1", from, to);
        LOG_F(ERROR, "Currency(v2): URL is {}", url);

        return {1.0, false};
    }

    if (res->status != 200) {
        LOG_F(ERROR, "Currency(v2): Error Response {}, setting exchange between {} to {} to 1/1", res->status, from,
              to);
        LOG_F(ERROR, "Currency(v2): URL is {}", url);
        LOG_F(ERROR, "Currency(v2): Response is {}", res->body);

        return {1.0, false};
    }

    auto& buffer = res->body;
    auto index = "\"" + to + "\":";

    if (buffer.find(index) == std::string::npos || buffer.find('}') == std::string::npos) {
        LOG_F(ERROR, "Currency(v2): Error parsing exchange rates, setting exchange between {} to {} to 1/1", from, to);
        LOG_F(ERROR, "Currency(v2): URL is {}", url);
        LOG_F(ERROR, "Currency(v2): Response is {}", res->body);

        return {1.0, false};
    }          std::string ratio_result(buffer.begin() + buffer.find(index) + index.size(), buffer.begin() + buffer.find('}'));

    return {atof(ratio_result.c_str()), true};
}

} // end of anonymous namespace

void budget::load_currency_cache(){
    const auto file_path = budget::path_to_budget_file("currency.cache");

    std::ifstream file(file_path);

    if (!file.is_open() || !file.good()){
        LOG_F(INFO, "Impossible to load Currency Cache");
        return;
    }

    std::string line;
    while (file.good() && getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        auto parts = splitv(line, ':');

        const currency_cache_key key(date_from_string(parts[0]), parts[1], parts[2]);
        exchanges[key] = {budget::to_number<double>(parts[3]), true};
    }

    LOG_F(INFO, "Share Price Cache has been loaded from {}", file_path.string());
    LOG_F(INFO, "Share Price Cache has {} entries", exchanges.size());
}

void budget::save_currency_cache() {
    const auto file_path = budget::path_to_budget_file("currency.cache");

    std::ofstream file(file_path);

    if (!file.is_open() || !file.good()){
        LOG_F(INFO, "Impossible to save Currency Cache");
        return;
    }

    {
        const server_lock_guard l(exchanges_lock);

        for (auto & [key, value] : exchanges) {
            // We only write down valid values
            if (value.valid) {
                file << key.date << ':' << key.from << ':' << key.to << ':' << value.value << std::endl;
            }
        }
    }

    LOG_F(INFO, "Share Price Cache has been loaded to {}", file_path.string());
    LOG_F(INFO, "Share Price Cache has {} entries", exchanges.size());
}

void budget::refresh_currency_cache(){
    std::unordered_map<currency_cache_key, currency_cache_value> copy;

    {
        const server_lock_guard l(exchanges_lock);

        copy = exchanges;
    }

    // Refresh/Prefetch the current exchange rates
    for (auto & [key, value] : copy) {
        exchange_rate(key.from, key.to);
    }

    LOG_F(INFO, "Currency Cache has been refreshed");
    LOG_F(INFO, "Currency Cache has {} entries", exchanges.size());
}

double budget::exchange_rate(const std::string& from){
    return exchange_rate(from, get_default_currency());
}

double budget::exchange_rate(const std::string& from, const std::string& to){
    return exchange_rate(from, to, budget::local_day());
}

double budget::exchange_rate(const std::string& from, const budget::date& d) {
    return exchange_rate(from, get_default_currency(), d);
}

double budget::exchange_rate(const std::string& from, const std::string& to, const budget::date& d) {
    assert(from != "DESIRED" && to != "DESIRED");

    if (from == to) {
        return 1.0;
    }
    if (d > budget::local_day()) {
        return exchange_rate(from, to, budget::local_day());
    }
    currency_cache_key key(d, from, to);

    // Return directly if we already have the data in cache
    {
        server_lock_guard l(exchanges_lock);

        if (exchanges.find(key) != exchanges.end()) {
            return exchanges[key].value;
        }
    }

    // Otherwise, make the API call without the lock

    auto rate = get_rate_v2(from, to, date_to_string(d));

    LOG_F(INFO, "Price: Currency Rate ({}) from {} to {} = {} (valid: {})", budget::to_string(d), from, to,
          budget::to_string(rate.value), rate.valid);

    // Update the cache and the reverse cache with the lock

    currency_cache_key reverse_key(d, to, from);

    {
        server_lock_guard l(exchanges_lock);

        exchanges[key] = rate;
        exchanges[reverse_key] = {1.0 / rate.value, rate.valid};
    }

    return rate.value;
}
