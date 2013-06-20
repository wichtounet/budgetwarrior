//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>
#include <string>

#include "money.hpp"

namespace budget {

int handle_debts(const std::vector<std::string>& args);

struct debt {
    int id;
    std::string guid;
    bool direction;
    std::string name;
    money amount;
    std::string title;
};

struct debts {
    int next_id;
    std::vector<debt> debts;
};

} //end of namespace budget
