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

    money(long dollars, int cents) {
        if (dollars < 0) {
            value = -(-dollars * SCALE + cents);
        } else {
            value = dollars * SCALE + cents;
        }
    }

    static money from_double(double dollars) {
        long dollars_long = static_cast<long>(dollars);
        return {dollars_long, static_cast<int>((dollars - dollars_long) * SCALE)};
    }

    money& operator=(long dollars){
        this->value = dollars * SCALE;

        return *this;
    }

    money operator+(const money& rhs) const {
        money new_money = *this;
        new_money += rhs;
        return new_money;
    }

    money& operator+=(const money& rhs){
        value += rhs.value;
        return *this;
    }

    money operator-(const money& rhs) const {
        money new_money = *this;
        new_money-= rhs;
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
        return *this += money(rhs);
    }

    money operator-(long rhs) const {
        return *this - money(rhs);
    }

    money& operator-=(long rhs){
        return *this -= money(rhs);
    }

    money operator+(int rhs) const {
        return *this + money(rhs);
    }

    money& operator+=(int rhs){
        return *this += money(rhs);
    }

    money operator-(int rhs) const {
        return *this - money(rhs);
    }

    money& operator-=(int rhs){
        return *this -= money(rhs);
    }

    money operator*(double factor) const {
        money new_money = *this;
        new_money *= factor;
        return new_money;
    }

    money& operator*=(double factor){
        value *= factor;
        return *this;
    }

    money operator*(float factor) const {
        money new_money = *this;
        new_money *= factor;
        return new_money;
    }

    money& operator*=(float factor){
        value *= factor;
        return *this;
    }

    money operator*(int factor) const {
        money new_money = *this;
        new_money *= factor;
        return new_money;
    }

    money& operator*=(int factor){
        value *= factor;
        return *this;
    }

    money operator/(int factor) const {
        money new_money = *this;
        new_money /= factor;
        return new_money;
    }

    money operator/(long factor) const {
        money new_money = *this;
        new_money /= factor;
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

    friend auto operator<=>(const budget::money & lhs, const budget::money & rhs) = default;

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

    explicit operator float() const {
        return value / float(SCALE);
    }

    explicit operator double() const {
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
money money_from_string(std::string_view money_string);

std::ostream& operator<<(std::ostream& stream, const money& amount);

money random_money(size_t min, size_t max);

std::string random_name(size_t length);

template<>
inline std::string to_string(money amount){
    std::stringstream stream;
    stream << amount;
    return stream.str();
}

} //end of namespace budget
