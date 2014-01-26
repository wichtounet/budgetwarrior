//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPUTE_H
#define COMPUTE_H

#include <boost/date_time/gregorian/gregorian.hpp>

#include "money.hpp"

namespace budget {

struct status {
    budget::money expenses;
    budget::money earnings;
    budget::money budget;
    budget::money balance;
};

status compute_year_status();
status compute_year_status(boost::gregorian::greg_year year);

} //end of namespace budget

#endif
