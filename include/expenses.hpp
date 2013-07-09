//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef EXPENSES_H
#define EXPENSES_H

#include <vector>
#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "money.hpp"

namespace budget {

typedef boost::gregorian::date date;

struct expense {
    std::size_t id;
    std::wstring guid;
    date expense_date;
    std::wstring name;
    std::wstring account;
    money amount;
};

std::wostream& operator<<(std::wostream& stream, const expense& expense);
void operator>>(const std::vector<std::wstring>& parts, expense& expense);

void load_expenses();
void save_expenses();
void show_expenses();
void show_expenses(boost::gregorian::greg_month month);
void show_expenses(boost::gregorian::greg_month month, boost::gregorian::greg_year year);
void show_all_expenses();

std::vector<expense>& all_expenses();

void handle_expenses(const std::vector<std::wstring>& args);

} //end of namespace budget

#endif
