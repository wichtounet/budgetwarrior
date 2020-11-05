//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

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

    explicit money(long dollars) : value(dollars * SCALE) {
        //Nothing to init
    }

    money(long dollars, int cents) : value(dollars * SCALE + cents) {
        //Nothing to init
    }

    money& operator=(long dollars){
        this->value = dollars * SCALE;

        return *this;
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

    money operator+(long rhs) const {
        return *this + money(rhs);
    }

    money& operator+=(long rhs){
        *this += money(rhs);
        return *this;
    }

    money operator-(long rhs) const {
        return *this - money(rhs);
    }

    money& operator-=(long rhs){
        *this -= money(rhs);
        return *this;
    }

    money operator+(int rhs) const {
        return *this + money(rhs);
    }

    money& operator+=(int rhs){
        *this += money(rhs);
        return *this;
    }

    money operator-(int rhs) const {
        return *this - money(rhs);
    }

    money& operator-=(int rhs){
        *this -= money(rhs);
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

    money operator*(float factor) const {
        money new_money = *this;
        new_money.value *= factor;
        return new_money;
    }

    money& operator*=(float factor){
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

    friend money operator*(double factor, money m){
        return m * factor;
    }

    friend money operator*(float factor, money m){
        return m * factor;
    }

    friend money operator*(int factor, money m){
        return m * factor;
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

    money abs() const {
        auto a = *this;
        if (a.value < 0) {
            a.value = -a.value;
        }
        return a;
    }
};

// Official money parsing functions
std::string money_to_string(const money& amount);
money money_from_string(const std::string& money_string);

std::ostream& operator<<(std::ostream& stream, const money& amount);

std::string to_flat_string(const money& amount);

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

template<typename Range, typename Functor>
inline budget::money accumulate_amount_if(const Range& range, Functor f){
    return accumulate_amount_if(range.begin(), range.end(), f);
}

template<typename InputIt>
inline budget::money accumulate_amount(InputIt first, InputIt last){
    budget::money init;

    for (; first != last; ++first) {
        init = init + first->amount;
    }

    return init;
}

template<typename Range>
inline budget::money accumulate_amount(const Range& range){
    return accumulate_amount(range.begin(), range.end());
}

} //end of namespace budget
