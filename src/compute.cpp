//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <utility>

#include "compute.hpp"
#include "expenses.hpp"
#include "earnings.hpp"

budget::status budget::compute_year_status(){
    auto today = boost::gregorian::day_clock::local_day();
    return compute_year_status(today.year());
}

budget::status budget::compute_year_status(boost::gregorian::greg_year year){
    budget::status status;



    return std::move(status);
}
