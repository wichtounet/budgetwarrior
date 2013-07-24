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

const int SCALE = 100;

struct money {
    long value;

    money() : value(0) {
        //Nothing to init
    }

    money(int dollars, int cents) : value(dollars * SCALE + cents) {
        //Nothing to init
    }

    money operator+(const money& rhs) const {
        money new_money = *this;
        new_money.value += rhs.value;
        return new_money;
    }

    money& operator+=(const money& rhs){
        value += rhs.value;
        return *this;
    }

    money operator-(const money& rhs) const {
        money new_money = *this;
        new_money.value -= rhs.value;
        return new_money;
    }

    money& operator-=(const money& rhs){
        value -= rhs.value;
        return *this;
    }

    money operator*(int factor) const {
        money new_money = *this;
        new_money.value *= factor;
        return new_money;
    }

    money& operator*=(int factor){
        value *= factor;
        return *this;
    }

    money operator/(int factor) const {
        money new_money = *this;
        new_money.value /= factor;
        return new_money;
    }

    money& operator/=(int factor){
        value /= factor;
        return *this;
    }

    int cents() const {
        return std::abs(value % SCALE);
    }

    int dollars() const {
        return value / SCALE;
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
