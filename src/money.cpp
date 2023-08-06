//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <stdexcept>
#include <random>
#include <charconv>
#include <array>

#include "money.hpp"
#include "utils.hpp"
#include "budget_exception.hpp"

using namespace budget;

money budget::money_from_string(std::string money_string){
    int dollars = 0;
    int cents = 0;

    if (auto [p, ec] = std::from_chars(money_string.data(), money_string.data() + money_string.size(), dollars); ec == std::errc()) {
        if (p == money_string.data() + money_string.size()) {
            return {dollars, cents};
        } else if(*p == '.') {
            ++p;

            if (auto [p2, ec] = std::from_chars(p, money_string.data() + money_string.size(), cents); ec == std::errc()) {
                if (p2 == money_string.data() + money_string.size()) {
                    if (cents >= 0 && cents < 100) {
                        return {dollars, cents};
                    }
                }
            }
        }
    }

    throw budget::budget_exception("\"" + money_string + "\" is not a valid amount of money");
}

std::string budget::money_to_string(const money& amount) {
    std::array<char, 128> buffer;

    auto p1 = buffer.begin();

    if (amount.negative()){
        *p1++ = '-';
    }

    if (auto [p2, ec] = std::to_chars(p1, buffer.end(), std::abs(amount.dollars())); ec == std::errc()) {
        *p2++ = '.';

        if (amount.cents() < 10){
            *p2++ = '0';
        }

        if (auto [p3, ec] = std::to_chars(p2, buffer.end(), amount.cents()); ec == std::errc()) {
            return {buffer.begin(), p3};
        } else {
            throw budget::budget_exception("money cant' be converted to string");
        }
    } else {
        throw budget::budget_exception("money cant' be converted to string");
    }
}

std::ostream& budget::operator<<(std::ostream& stream, const money& amount){
    return stream << money_to_string(amount);
}

budget::money budget::random_money(size_t min, size_t max){
    static std::random_device rd;
    static std::mt19937_64 engine(rd());

    std::uniform_int_distribution<int> dollars_dist(min, max);
    std::uniform_int_distribution<int> cents_dist(0, 99);

    return budget::money(dollars_dist(engine), cents_dist(engine));
}

std::string budget::random_name(size_t length){
    static std::random_device rd;
    static std::mt19937_64 engine(rd());

    std::uniform_int_distribution<int> letters_dist(0, 25);

    std::string name;

    for(size_t i = 0; i < length; ++i){
        name += 'A' + letters_dist(engine);
    }

    return name;
}
