//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "money.hpp"
#include "utils.hpp"
#include "budget_exception.hpp"

using namespace budget;

money budget::parse_money(const std::string& money_string){
    std::size_t dot_pos = money_string.find(".");

    int dollars = 0;
    int cents = 0;

    if(dot_pos == std::string::npos){
        dollars = to_number<int>(money_string);
    } else {
        dollars = to_number<int>(money_string.substr(0, dot_pos));

        auto cents_str = money_string.substr(dot_pos+1, money_string.size() - dot_pos);
        cents = to_number<int>(cents_str);
    }

    return {dollars, cents};
}

std::ostream& budget::operator<<(std::ostream& stream, const money& amount){
    if(amount.cents() < 10){
        return stream << amount.dollars() << ".0" << amount.cents();
    } else {
        return stream << amount.dollars() << "." << amount.cents();
    }
}

void budget::not_negative(const money& amount){
    if(amount.dollars() < 0 || amount.cents() < 0){
        throw budget_exception("Amount cannot be negative");
    }
}
