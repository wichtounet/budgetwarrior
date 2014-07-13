//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef DATE_HPP
#define DATE_HPP

#include <ctime>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "utils.hpp"

namespace budget {

typedef boost::gregorian::date date;
typedef boost::gregorian::greg_year year;
typedef boost::gregorian::greg_month month;
typedef boost::gregorian::greg_day day;

date local_day();

inline date from_string(const std::string& str){
    return boost::gregorian::from_string(str);
}

inline date from_iso_string(const std::string& str){
    auto y = year(to_number<unsigned short>(str.substr(0, 4)));
    auto m = month(to_number<unsigned short>(str.substr(4, 2)));
    auto d = day(to_number<unsigned short>(str.substr(6, 2)));

    return {y, m, d};
}

inline std::string date_to_string(date date){
    return boost::gregorian::to_iso_extended_string(date);
}

template<>
inline std::string to_string(budget::date date){
    return date_to_string(date);
}

unsigned short start_year();
unsigned short start_month(budget::year year);

} //end of namespace budget

#endif
