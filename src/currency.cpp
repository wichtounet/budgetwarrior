//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <map>
#include <utility>
#include <iostream>

#include "currency.hpp"

#include <curl/curl.h>

using namespace budget;

namespace {

std::map<std::pair<std::string, std::string>, double> exchanges;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

} // end of anonymous namespace

double budget::exchange_rate(const std::string& from, const std::string& to){
    if(from == to){
        return 1.0;
    } else {
        auto key = std::make_pair(from, to);
        auto reverse_key = std::make_pair(to, from);

        if (!exchanges.count(key)) {
            auto url = "http://free.currencyconverterapi.com/api/v3/convert?q=" + from + "_" + to + "&compact=ultra";
            CURL* curl;
            CURLcode res;
            std::string buffer;

            curl = curl_easy_init();
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                if (res) {
                    std::cout << "Error accessing exchange rates, setting exchange between " << from << " to " << to << " to 1/1" << std::endl;

                    exchanges[key] = 1.0;
                } else {
                    if (buffer.find(':') == std::string::npos || buffer.find('}') == std::string::npos) {
                        std::cout << "Error parsing exchange rates, setting exchange between " << from << " to " << to << " to 1/1" << std::endl;

                        exchanges[key] = 1.0;
                    } else {
                        std::string ratio_result(buffer.begin() + buffer.find(':') + 1, buffer.begin() + buffer.find('}'));

                        exchanges[key] = atof(ratio_result.c_str());
                        exchanges[reverse_key] = 1.0 / exchanges[key];
                    }
                }
            }
        }

        return exchanges[key];
    }
}
