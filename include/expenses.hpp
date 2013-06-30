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
    std::string guid;
    date expense_date;
    std::string name;
    money amount;
};

std::ostream& operator<<(std::ostream& stream, const expense& expense);
void operator>>(const std::vector<std::string>& parts, expense& expense);

void load_expenses();
void save_expenses();
void show_expenses();
void show_expenses(boost::gregorian::greg_month month);
void show_expenses(boost::gregorian::greg_month month, boost::gregorian::greg_year year);
void all_expenses();

int handle_expenses(const std::vector<std::string>& args);

} //end of namespace budget

#endif
