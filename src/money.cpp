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

money budget::parse_money(const std::wstring& money_string){
    money amount;

    std::size_t dot_pos = money_string.find(L".");

    if(dot_pos == std::wstring::npos){
        amount.dollars = to_number<int>(money_string);
    } else {
        amount.dollars = to_number<int>(money_string.substr(0, dot_pos));
        amount.cents = to_number<int>(money_string.substr(dot_pos+1, money_string.size() - dot_pos));
    }

    return amount;
}

std::wostream& budget::operator<<(std::wostream& stream, const money& amount){
    return stream << amount.dollars << L"." << amount.cents;
}

void budget::not_negative(const money& amount){
    if(amount.dollars < 0 || amount.cents < 0){
        throw budget_exception(L"Amount cannot be negative");
    }
}
