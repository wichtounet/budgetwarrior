//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "utils.hpp"
#include "budget_exception.hpp"

void budget::not_empty(const std::string& value, const std::string& message){
    if(value.empty()){
        throw budget_exception(message);
    }
}
