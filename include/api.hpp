//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <string>
#include <utility>
#include <map>

namespace budget {

struct api_response {
    bool success;
    std::string result;
};

api_response api_get(const std::string& api);
api_response api_post(const std::string& api, const std::map<std::string, std::string, std::less<>>& params);

} //end of namespace budget
