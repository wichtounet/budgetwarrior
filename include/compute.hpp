//=======================================================================
// Copyright (c) 2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
