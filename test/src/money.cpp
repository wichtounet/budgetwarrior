//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "test.hpp"
#include "money.hpp"
#include "budget_exception.hpp"

using namespace std::string_literals;

TEST_CASE("money/to_string") {
    budget::money a(55);

    FAST_CHECK_EQ(budget::money_to_string(a), "55.00"s);
    FAST_CHECK_EQ(budget::to_string(a), "55.00"s);

    budget::money b(55, 55);

    FAST_CHECK_EQ(budget::money_to_string(b), "55.55"s);
    FAST_CHECK_EQ(budget::to_string(b), "55.55"s);

    budget::money c(123456, 55);

    FAST_CHECK_EQ(budget::money_to_string(c), "123456.55"s);
    FAST_CHECK_EQ(budget::to_string(c), "123456.55"s);

    budget::money d(0, 5);

    FAST_CHECK_EQ(budget::money_to_string(d), "0.05"s);
    FAST_CHECK_EQ(budget::to_string(d), "0.05"s);

    budget::money e(-1, 5);

    FAST_CHECK_EQ(budget::money_to_string(e), "-1.05"s);
    FAST_CHECK_EQ(budget::to_string(e), "-1.05"s);
}

TEST_CASE("money/money_from_string/1") {
    auto a = budget::money_from_string("100");
    auto b = budget::money_from_string("100.0");
    auto c = budget::money_from_string("100.56");
    auto d = budget::money_from_string("100.06");
    auto e = budget::money_from_string("100.6");
    auto f = budget::money_from_string("10000.60");
    auto g = budget::money_from_string("10,000.60");
    auto h = budget::money_from_string("1,100,000.60");

    FAST_CHECK_EQ(a, budget::money(100));
    FAST_CHECK_EQ(b, budget::money(100));
    FAST_CHECK_EQ(c, budget::money(100, 56));
    FAST_CHECK_EQ(d, budget::money(100, 6));
    FAST_CHECK_EQ(e, budget::money(100, 6));
    FAST_CHECK_EQ(f, budget::money(10000, 60));
    FAST_CHECK_EQ(g, budget::money(10000, 60));
    FAST_CHECK_EQ(h, budget::money(1100000, 60));
}

TEST_CASE("money/money_from_string/2") {
    REQUIRE_THROWS_AS(budget::money_from_string("100.100"), budget::budget_exception);
    REQUIRE_THROWS_AS(budget::money_from_string("10.0.0"), budget::budget_exception);
    REQUIRE_THROWS_AS(budget::money_from_string(".0"), budget::budget_exception);
    REQUIRE_THROWS_AS(budget::money_from_string("100'000.0"), budget::budget_exception);
    REQUIRE_THROWS_AS(budget::money_from_string("100.-100"), budget::budget_exception);
}

// Test operator + -
// Test operator *
// Test operator /
// Test comparisons
// Test conversions
// Test abs
