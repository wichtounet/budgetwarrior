//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <tuple>
#include <utility>
#include <iostream>

#include "currency.hpp"
#include "server.hpp"
#include "assets.hpp" // For get_default_currency
#include "http.hpp"
#include "date.hpp"
#include "config.hpp"
#include "server_lock.hpp"

namespace {

struct currency_cache_key {
    budget::date date;
    std::string from;
    std::string to;

    currency_cache_key(budget::date date, std::string from, std::string to) : date(date), from(from), to(to) {}

    friend bool operator==(const currency_cache_key & lhs, const currency_cache_key & rhs){
        if (lhs.date != rhs.date) {
            return false;
        }

        if (lhs.from != rhs.from) {
            return false;
        }

        return lhs.to == rhs.to;
    }
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

std::unordered_map<currency_cache_key, double> exchanges;
budget::server_lock exchanges_lock;

// V1 is using free.currencyconverterapi.com
double get_rate_v1(const std::string& from, const std::string& to){
    httplib::Client cli("free.currencyconverterapi.com", 80);

    std::string api_complete = "/api/v3/convert?q=" + from + "_" + to + "&compact=ultra";

    auto res = cli.Get(api_complete.c_str());

    if (!res) {
        std::cout << "Error accessing exchange rates (no response), setting exchange between " << from << " to " << to << " to 1/1" << std::endl;

        return  1.0;
    } else if (res->status != 200) {
        std::cout << "Error accessing exchange rates (not OK), setting exchange between " << from << " to " << to << " to 1/1" << std::endl;

        return  1.0;
    } else {
        auto& buffer = res->body;

        if (buffer.find(':') == std::string::npos || buffer.find('}') == std::string::npos) {
            std::cout << "Error parsing exchange rates, setting exchange between " << from << " to " << to << " to 1/1" << std::endl;

            return  1.0;
        } else {
            std::string ratio_result(buffer.begin() + buffer.find(':') + 1, buffer.begin() + buffer.find('}'));

            return atof(ratio_result.c_str());
        }
    }
}

// V2 is using api.exchangeratesapi.io
double get_rate_v2(const std::string& from, const std::string& to, const std::string& date = "latest") {
    httplib::SSLClient cli("api.exchangeratesapi.io", 443);

    std::string api_complete = "/" + date + "?symbols=" + to + "&base=" + from;

    auto res = cli.Get(api_complete.c_str());

    if (!res) {
        std::cout << "ERROR: Currency(v2): No response, setting exchange between " << from << " to " << to << " to 1/1" << std::endl;
        std::cout << "ERROR: Currency(v2): URL is " << api_complete << std::endl;

        return  1.0;
    } else if (res->status != 200) {
        std::cout << "ERROR: Currency(v2): Error response " << res->status << ", setting exchange between " << from << " to " << to << " to 1/1" << std::endl;
        std::cout << "ERROR: Currency(v2): URL is " << api_complete << std::endl;
        std::cout << "ERROR: Currency(v2): Response is " << res->body << std::endl;

        return  1.0;
    } else {
        auto& buffer = res->body;
        auto index   = "\"" + to + "\":";

        if (buffer.find(index) == std::string::npos || buffer.find('}') == std::string::npos) {
            std::cout << "ERROR: Currency(v2): Error parsing exchange rates, setting exchange between " << from << " to " << to << " to 1/1" << std::endl;
            std::cout << "ERROR: Currency(v2): URL is " << api_complete << std::endl;
            std::cout << "ERROR: Currency(v2): Response is " << res->body << std::endl;

            return  1.0;
        } else {
            std::string ratio_result(buffer.begin() + buffer.find(index) + index.size(), buffer.begin() + buffer.find('}'));

            return atof(ratio_result.c_str());
        }
    }
}

} // end of anonymous namespace

void budget::load_currency_cache(){
    std::string file_path = budget::path_to_budget_file("currency.cache");
    std::ifstream file(file_path);

    if (!file.is_open() || !file.good()){
        std::cout << "INFO: Impossible to load Currency Cache" << std::endl;
        return;
    }

    std::string line;
    while (file.good() && getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        auto parts = split(line, ':');

        currency_cache_key key(date_from_string(parts[0]), parts[1], parts[2]);
        exchanges[key] = budget::to_number<double>(parts[3]);
    }

    if (budget::is_server_running()) {
        std::cout << "INFO: Currency Cache has been loaded from " << file_path << std::endl;
        std::cout << "INFO: Currency Cache has " << exchanges.size() << " entries " << std::endl;
    }
}

void budget::save_currency_cache() {
    std::string file_path = budget::path_to_budget_file("currency.cache");
    std::ofstream file(file_path);

    if (!file.is_open() || !file.good()){
        std::cout << "INFO: Impossible to save Currency Cache" << std::endl;
        return;
    }

    {
        server_lock_guard l(exchanges_lock);

        for (auto & [key, value] : exchanges) {
            if (value != 1.0) {
                file << key.date << ':' << key.from << ':' << key.to << ':' << value << std::endl;
            }
        }
    }

    if (budget::is_server_running()) {
        std::cout << "INFO: Currency Cache has been saved to " << file_path << std::endl;
        std::cout << "INFO: Currency Cache has " << exchanges.size() << " entries " << std::endl;
    }
}

void budget::refresh_currency_cache(){
    std::unordered_map<currency_cache_key, double> copy;

    {
        server_lock_guard l(exchanges_lock);

        copy = exchanges;
    }

    // Refresh/Prefetch the current exchange rates
    for (auto & pair : copy) {
        auto& key = pair.first;

        exchange_rate(key.from, key.to);
    }

    if (budget::is_server_running()) {
        std::cout << "INFO: Currency Cache has been refreshed" << std::endl;
        std::cout << "INFO: Currency Cache has " << exchanges.size() << " entries " << std::endl;
    }
}

double budget::exchange_rate(const std::string& from){
    return exchange_rate(from, get_default_currency());
}

double budget::exchange_rate(const std::string& from, const std::string& to){
    return exchange_rate(from, to, budget::local_day());
}

double budget::exchange_rate(const std::string& from, budget::date d){
    return exchange_rate(from, get_default_currency(), d);
}

double budget::exchange_rate(const std::string& from, const std::string& to, budget::date d){
    assert(from != "DESIRED" && to != "DESIRED");

    if (from == to) {
        return 1.0;
    } else {
        currency_cache_key key(d, from, to);
        currency_cache_key reverse_key(d, to, from);

        // Return directly if we already have the data in cache
        {
            server_lock_guard l(exchanges_lock);

            if (exchanges.count(key)) {
                return exchanges[key];
            }
        }

        // Otherwise, make the API call without the call

        auto rate = get_rate_v2(from, to, date_to_string(d));

        if (budget::is_server_running()) {
            std::cout << "INFO: Currency: Rate (" << d << ")"
                << " from " << from << " to " << to << " = " << rate << std::endl;
        }

        // Update the cache

        {
            server_lock_guard l(exchanges_lock);

            exchanges[key]         = rate;
            exchanges[reverse_key] = 1.0 / rate;
        }

        return rate;
    }
}
