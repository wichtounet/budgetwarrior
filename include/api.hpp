//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef API_H
#define API_H

#include <string>
#include <utility>
#include <map>

namespace budget {

struct api_response {
    bool success;
    std::string result;
};

api_response api_get(const std::string& api);
api_response api_post(const std::string& api, const std::map<std::string, std::string>& params);

} //end of namespace budget

#endif
