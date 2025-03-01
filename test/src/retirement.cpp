//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "test.hpp"
#include "money.hpp"
#include "retirement.hpp"

using namespace std::string_literals;

TEST_CASE("retirement/fixed_fi_ratio/1") {
    const double        wrate(4.0);
    const budget::money expenses(40000);
    const budget::money nw(1000000);

    FAST_CHECK_EQ(budget::fixed_fi_ratio(wrate, nw, expenses), 1.0);
}

TEST_CASE("retirement/fixed_fi_ratio/2") {
    const double        wrate(4.0);
    const budget::money expenses(100000);
    const budget::money nw(1000000);

    FAST_CHECK_EQ(budget::fixed_fi_ratio(wrate, nw, expenses), 0.4);
}

TEST_CASE("retirement/fixed_fi_ratio/3") {
    const double        wrate(4.0);
    const budget::money expenses(100000);
    const budget::money nw(3000000);

    FAST_CHECK_EQ(budget::fixed_fi_ratio(wrate, nw, expenses), 1.2);
}
