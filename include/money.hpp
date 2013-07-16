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
        money new_money = *this;
        new_money += rhs;
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

    money operator-(const money& rhs) const {
        money new_money = *this;
        new_money -= rhs;
        return new_money;
    }

    money& operator-=(const money& rhs){
        dollars = dollars - rhs.dollars;
        cents = cents - rhs.cents;

        if(cents < 0){
            dollars -= cents / 100;
            cents %= 100;
            cents *= -1;
        }

        return *this;
    }

    money operator*(int factor) const {
        money new_money = *this;
        new_money *= factor;
        return new_money;
    }

    money& operator*=(int factor){
        dollars = dollars * factor;
        cents = cents * factor;

        if(cents > 100){
            dollars += cents / 100;
            cents %= 100;
        }

        if(cents < 0){
            dollars -= cents / 100;
            cents %= 100;
            cents *= -1;
        }

        return *this;
    }

    money operator/(int factor) const {
        money new_money = *this;
        new_money /= factor;
        return new_money;
    }

    money& operator/=(int factor){
        dollars = dollars / factor;
        cents = cents / factor;

        //TODO The cents of the dollars should be reported to cents

        return *this;
    }
};

std::ostream& operator<<(std::ostream& stream, const money& amount);

money parse_money(const std::string& money_string);
void not_negative(const money& money);

template<>
inline std::string to_string(const money& amount){
    std::stringstream stream;
    stream << amount;
    return stream.str();
}

} //end of namespace budget

#endif
