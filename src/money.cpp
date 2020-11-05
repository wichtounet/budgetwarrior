//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <stdexcept>
#include <random>

#include "money.hpp"
#include "utils.hpp"
#include "budget_exception.hpp"

using namespace budget;

money budget::money_from_string(const std::string& money_string){
    size_t dot_pos = money_string.rfind(".");

    int dollars = 0;
    int cents = 0;

    try {
        if(dot_pos == std::string::npos){
            dollars = to_number<int>(money_string);
        } else {
            dollars = to_number<int>(money_string.substr(0, dot_pos));

            auto cents_str = money_string.substr(dot_pos+1, money_string.size() - dot_pos);
            cents = to_number<int>(cents_str);
        }
    } catch (std::invalid_argument& e){
        throw budget::budget_exception("\"" + money_string + "\" is not a valid amount of money");
    } catch (std::out_of_range& e){
        throw budget::budget_exception("\"" + money_string + "\" is not a valid amount of money");
    }

    return {dollars, cents};
}

std::string budget::money_to_string(const money& amount) {
    std::stringstream stream;
    stream << amount;
    return stream.str();
}

std::ostream& budget::operator<<(std::ostream& stream, const money& amount){
    if(amount.cents() < 10){
        if(amount.negative()){
            return stream << '-' << (-1 * amount.dollars()) << ".0" << amount.cents();
        } else {
            return stream << amount.dollars() << ".0" << amount.cents();
        }
    } else {
        if(amount.negative()){
            return stream << '-' << (-1 * amount.dollars()) << "." << amount.cents();
        } else {
            return stream << amount.dollars() << "." << amount.cents();
        }
    }
}

std::string budget::to_flat_string(const money& amount){
    std::stringstream stream;
    stream.imbue(std::locale("C"));

    if (amount.cents() < 10) {
        if (amount.negative()) {
            stream << '-' << (-1 * amount.dollars()) << ".0" << amount.cents();
        } else {
            stream << amount.dollars() << ".0" << amount.cents();
        }
    } else {
        if (amount.negative()) {
            stream << '-' << (-1 * amount.dollars()) << "." << amount.cents();
        } else {
            stream << amount.dollars() << "." << amount.cents();
        }
    }

    return stream.str();
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
