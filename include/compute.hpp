//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
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

    status add_expense(budget::money expense){
        auto new_status = *this;

        new_status.expenses += expense;
        new_status.balance -= expense;

        return std::move(new_status);
    }

    status add_earning(budget::money earning){
        auto new_status = *this;

        new_status.earnings += earnings;
        new_status.balance += earnings;

        return std::move(new_status);
    }
};

status compute_year_status();
status compute_year_status(boost::gregorian::greg_year year);
status compute_year_status(boost::gregorian::greg_year year, boost::gregorian::greg_month last);

status compute_month_status();
status compute_month_status(boost::gregorian::greg_month year);
status compute_month_status(boost::gregorian::greg_year year, boost::gregorian::greg_month month);

} //end of namespace budget

#endif
