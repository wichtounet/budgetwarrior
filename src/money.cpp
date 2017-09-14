//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <stdexcept>

#include "money.hpp"
#include "utils.hpp"
#include "budget_exception.hpp"

using namespace budget;

money budget::parse_money(const std::string& money_string){
    size_t dot_pos = money_string.find(".");

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
