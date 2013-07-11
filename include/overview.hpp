//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <vector>
#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

namespace budget {

void month_overview();
void month_overview(boost::gregorian::greg_month month);
void month_overview(boost::gregorian::greg_month month, boost::gregorian::greg_year year);

void year_overview();
void year_overview(boost::gregorian::greg_year year);

void handle_overview(const std::vector<std::wstring>& args);

} //end of namespace budget

#endif
