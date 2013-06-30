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

#include <boost/date_time/posix_time/posix_time.hpp>

#include "money.hpp"

namespace budget {

typedef boost::posix_time::ptime time;

struct expense {
    std::size_t id;
    std::string guid;
    time expense_time;
    std::string name;
    money amount;
};

std::ostream& operator<<(std::ostream& stream, const expense& expense);
void operator>>(const std::vector<std::string>& parts, expense& expense);

void load_expenses();
void save_expenses();
void show_expenses();

int handle_expenses(const std::vector<std::string>& args);

} //end of namespace budget

#endif
