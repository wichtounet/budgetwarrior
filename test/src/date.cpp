//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "test.hpp"
#include "date.hpp"

TEST_CASE("date/minus/days") {
    budget::date a(2010, 5, 6);
    a -= budget::days(1);

    FAST_CHECK_EQ(a.year(), 2010);
    FAST_CHECK_EQ(a.month(), 5);
    FAST_CHECK_EQ(a.day(), 5);
}
