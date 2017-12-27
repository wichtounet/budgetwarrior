//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MONEY_H
#define MONEY_H

#include <string>
#include <ostream>

#include "utils.hpp"

namespace budget {

constexpr const int SCALE = 100;

struct money {
    long value;

    money() : value(0) {
        //Nothing to init
    }

    explicit money(int dollars) : value(dollars * SCALE) {
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

    money operator*(double factor) const {
        money new_money = *this;
        new_money.value *= factor;
        return new_money;
    }

    money& operator*=(double factor){
        value *= factor;
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

    money operator/(long factor) const {
        money new_money = *this;
        new_money.value /= factor;
        return new_money;
    }

    money& operator/=(int factor){
        value /= factor;
        return *this;
    }

    double operator/(money m) const {
        return value / double(m.value);
    }

    bool operator==(const budget::money& rhs) const {
        return value == rhs.value;
    }

    bool operator!=(const budget::money& rhs) const {
        return value != rhs.value;
    }

    bool operator<(const budget::money& rhs) const {
        return value < rhs.value;
    }

    bool operator<=(const budget::money& rhs) const {
        return value < rhs.value;
    }

    bool operator>(const budget::money& rhs) const {
        return value > rhs.value;
    }

    bool operator>=(const budget::money& rhs) const {
        return value >= rhs.value;
    }

    int cents() const {
        return std::abs(value % SCALE);
    }

    int dollars() const {
        return value / SCALE;
    }

    bool positive() const {
        return value > 0;
    }

    bool negative() const {
        return value < 0;
    }

    bool zero() const {
        return value == 0;
    }

    operator bool() const {
        return value;
    }

    operator float() const {
        return value / float(SCALE);
    }

    operator double() const {
        return value / double(SCALE);
    }
};

std::ostream& operator<<(std::ostream& stream, const money& amount);

money parse_money(const std::string& money_string);

money random_money(size_t min, size_t max);

std::string random_name(size_t length);

template<>
inline std::string to_string(money amount){
    std::stringstream stream;
    stream << amount;
    return stream.str();
}

template<typename InputIt, typename Functor>
inline budget::money accumulate_amount_if(InputIt first, InputIt last, Functor f){
    budget::money init;

    for (; first != last; ++first) {
        if(f(*first)){
            init = init + first->amount;
        }
    }

    return init;
}

template<typename T, typename Functor>
inline budget::money accumulate_amount_if(std::vector<T>& container, Functor f){
    return accumulate_amount_if(container.begin(), container.end(), f);
}

} //end of namespace budget

#endif
