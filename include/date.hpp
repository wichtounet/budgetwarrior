//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef DATE_HPP
#define DATE_HPP

#include <boost/date_time/gregorian/gregorian.hpp>

namespace budget {

typedef boost::gregorian::date date;
typedef boost::gregorian::greg_year year;
typedef boost::gregorian::greg_month month;

inline date local_day(){
    return boost::gregorian::day_clock::local_day();
}

} //end of namespace budget

#endif
