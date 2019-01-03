//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
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

namespace {

std::map<std::tuple<std::string, std::string>, double> exchanges;

// V1 is using free.currencyconverterapi.com
double get_rate_v1(const std::string& from, const std::string& to){
    httplib::Client cli("free.currencyconverterapi.com", 80);

    std::string api_complete = "/api/v3/convert?q=" + from + "_" + to + "&compact=ultra";

    auto res = cli.get(api_complete.c_str());

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
    httplib::Client cli("api.exchangeratesapi.io", 80);

    std::string api_complete = date + "?symbols=" + to + "&base=" + from;

    auto res = cli.get(api_complete.c_str());

    if (!res) {
        std::cout << "ERROR: Currency(v2): No response, setting exchange between " << from << " to " << to << " to 1/1" << std::endl;
        std::cout << "ERROR: Currency(v2): URL is" << api_complete << std::endl;

        return  1.0;
    } else if (res->status != 200) {
        std::cout << "ERROR: Currency(v2): Error response " << res->status << ", setting exchange between " << from << " to " << to << " to 1/1" << std::endl;
        std::cout << "ERROR: Currency(v2): URL is" << api_complete << std::endl;
        std::cout << "ERROR: Currency(v2): Response is" << res->body << std::endl;

        return  1.0;
    } else {
        auto& buffer = res->body;

        if (buffer.find("\"" + to + "\":") == std::string::npos || buffer.find('}') == std::string::npos) {
            std::cout << "ERROR: Currency(v2): Error parsing exchange rates, setting exchange between " << from << " to " << to << " to 1/1" << std::endl;
            std::cout << "ERROR: Currency(v2): URL is" << api_complete << std::endl;
            std::cout << "ERROR: Currency(v2): Response is" << res->body << std::endl;

            return  1.0;
        } else {
            std::string ratio_result(buffer.begin() + buffer.find("\"" + to + "\":") + 1, buffer.begin() + buffer.find('}'));

            return atof(ratio_result.c_str());
        }
    }
}

} // end of anonymous namespace

void budget::invalidate_currency_cache(){
    exchanges.clear();
}

double budget::exchange_rate(const std::string& from){
    return exchange_rate(from, get_default_currency());
}

double budget::exchange_rate(const std::string& from, const std::string& to){
    if(from == to){
        return 1.0;
    } else {
        auto key = std::make_tuple(from, to);
        auto reverse_key = std::make_tuple(to, from);

        if (!exchanges.count(key)) {
            auto rate = get_rate_v2(from, to);

            if (budget::is_server_running()) {
                std::cout << "INFO: Currency: Rate from " << from << " to " << to << " = " << rate << std::endl;
            }

            exchanges[key]         = rate;
            exchanges[reverse_key] = 1.0 / rate;
        }

        return exchanges[key];
    }
}
