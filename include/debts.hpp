//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEBTS_H
#define DEBTS_H

#include <vector>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "money.hpp"

namespace budget {

typedef boost::posix_time::ptime time;

struct debt {
    std::size_t id;
    int state;
    std::string guid;
    time creation_time;
    bool direction;
    std::string name;
    money amount;
    std::string title = "";
};

std::ostream& operator<<(std::ostream& stream, const debt& debt);
void operator>>(const std::vector<std::string>& parts, debt& debt);

void load_debts();
void save_debts();
void add_debt(debt&& debt);
void list_debts();
void all_debts();

int handle_debts(const std::vector<std::string>& args);

} //end of namespace budget

#endif
