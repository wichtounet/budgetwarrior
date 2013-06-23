//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MONEY_H
#define MONEY_H

#include <string>
#include <ostream>

#include "utils.hpp"

namespace budget {

struct money {
    int dollars = 0;
    int cents = 0;
};

std::ostream& operator<<(std::ostream& stream, const money& amount);

money parse_money(const std::string& money_string);

template<>
inline std::string to_string(money amount){
    std::stringstream stream;
    stream << amount;
    return stream.str();
}

} //end of namespace budget

#endif
