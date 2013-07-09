//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "utils.hpp"

using namespace budget;

std::wstring budget::to_wstring(const std::string& source){
    std::wstring result;
    result.assign(source.begin(), source.end());
    return result;
}

std::string budget::to_nstring(const std::wstring& source){
    std::string result;
    result.assign(source.begin(), source.end());
    return result;
}
