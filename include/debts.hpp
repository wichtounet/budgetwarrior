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
    int id;
    std::string guid;
    time creation_time;
    bool direction;
    std::string name;
    money amount;
    std::string title = "";
};

struct debts {
    int next_id;
    std::vector<debt> debts;
};

void load_debts();
void save_debts();
void add_debt(debt&& debt);

int handle_debts(const std::vector<std::string>& args);

} //end of namespace budget

#endif
