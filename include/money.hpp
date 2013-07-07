//=======================================================================
// Copyright Baptiste Wicht 2013.
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

    money operator+(const money& rhs) const {
        money new_money;
        new_money.dollars = dollars + rhs.dollars;
        new_money.cents = cents + rhs.cents;

        if(new_money.cents > 100){
            new_money.dollars += new_money.cents / 100;
            new_money.cents %= 100;
        }

        return new_money;
    }

    money& operator+=(const money& rhs){
        dollars += rhs.dollars;
        cents += rhs.cents;

        if(cents > 100){
            dollars += cents / 100;
            cents %= 100;
        }

        return *this;
    }
};

std::ostream& operator<<(std::ostream& stream, const money& amount);

money parse_money(const std::string& money_string);
void not_negative(const money& money);

template<>
inline std::string to_string(money amount){
    std::stringstream stream;
    stream << amount;
    return stream.str();
}

} //end of namespace budget

#endif
