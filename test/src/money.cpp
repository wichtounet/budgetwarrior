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

TEST_CASE("money/abs/1") {
    budget::money a(1, 11);
    budget::money b(-1, 11);
    budget::money c(-1000, 0);

    FAST_CHECK_EQ(a.abs(), budget::money(1, 11));
    FAST_CHECK_EQ(b.abs(), budget::money(1, 11));
    FAST_CHECK_EQ(c.abs(), budget::money(1000, 0));
}

TEST_CASE("money/plus/1") {
    budget::money a(100, 10);

    a += budget::money(99, 50);
    FAST_CHECK_EQ(a, budget::money(199, 60));

    a += 111;
    FAST_CHECK_EQ(a, budget::money(310, 60));

    FAST_CHECK_EQ(a + budget::money(25, 65), budget::money(336, 25));
    FAST_CHECK_EQ(a + 25, budget::money(335, 60));

    a += budget::money(-400, 50);
    FAST_CHECK_EQ(a, budget::money(-89, 90));
}

TEST_CASE("money/minus/1") {
    budget::money a(100, 10);

    a -= budget::money(39, 50);
    FAST_CHECK_EQ(a, budget::money(60, 60));

    a -= 100;
    FAST_CHECK_EQ(a, budget::money(-39, 40));

    FAST_CHECK_EQ(a - budget::money(25, 65), budget::money(-65, 5));
    FAST_CHECK_EQ(a - 25, budget::money(-64, 40));

    FAST_CHECK_EQ(a - budget::money(-10, 10), budget::money(-29, 30));
    FAST_CHECK_EQ(a - -10, budget::money(-29, 40));
}

TEST_CASE("money/factor/1") {
    budget::money a(100, 10);

    a *= 2;
    FAST_CHECK_EQ(a, budget::money(200, 20));

    a *= 1.2;
    FAST_CHECK_EQ(a, budget::money(240, 24));

    a *= -1;
    FAST_CHECK_EQ(a, budget::money(-240, 24));
}

TEST_CASE("money/factor/2") {
    budget::money a(100, 10);

    a /= 2;
    FAST_CHECK_EQ(a, budget::money(50, 5));

    a /= -1;
    FAST_CHECK_EQ(a, budget::money(-50, 5));
}

TEST_CASE("money/divide/1") {
    budget::money a(100, 0);
    budget::money b(20, 0);

    FAST_CHECK_EQ(a / b, 5.0);
}

TEST_CASE("money/divide/2") {
    budget::money a(100, 12);
    budget::money b(20, 24);

    FAST_CHECK_EQ(a / b, (10012.0 / 2024.0));
}

TEST_CASE("money/conversions/bool/1") {
    budget::money a(100, 10);
    budget::money b(-100, 10);
    budget::money c(0, 10);
    budget::money d(0, 0);

    FAST_CHECK_UNARY(static_cast<bool>(a));
    FAST_CHECK_UNARY(static_cast<bool>(b));
    FAST_CHECK_UNARY(static_cast<bool>(c));
    FAST_CHECK_UNARY(!static_cast<bool>(d));
}

TEST_CASE("money/conversions/float/1") {
    budget::money a(100, 10);
    budget::money b(-100, 10);
    budget::money c(0, 10);
    budget::money d(0, 0);

    FAST_CHECK_EQ(static_cast<float>(a), 100.10f);
    FAST_CHECK_EQ(static_cast<float>(b), -100.10f);
    FAST_CHECK_EQ(static_cast<float>(c), 0.10f);
    FAST_CHECK_EQ(static_cast<float>(d), 0.0f);
}

TEST_CASE("money/conversions/double/1") {
    budget::money a(100, 10);
    budget::money b(-100, 10);
    budget::money c(0, 10);
    budget::money d(0, 0);

    FAST_CHECK_EQ(static_cast<double>(a), 100.10);
    FAST_CHECK_EQ(static_cast<double>(b), -100.10);
    FAST_CHECK_EQ(static_cast<double>(c), 0.10);
    FAST_CHECK_EQ(static_cast<double>(d), 0.0);
}

TEST_CASE("money/comparisons/1") {
    budget::money a(1, 1);
    budget::money b(1, 10);
    budget::money c(0, 0);
    budget::money d(1, 1);

    FAST_CHECK_UNARY(a == a);
    FAST_CHECK_UNARY(b == b);
    FAST_CHECK_UNARY(c == c);

    FAST_CHECK_UNARY(a == d);
    FAST_CHECK_UNARY(!(a != d));

    FAST_CHECK_UNARY(!(a != a));
    FAST_CHECK_UNARY(!(b != b));
    FAST_CHECK_UNARY(!(c != c));

    FAST_CHECK_UNARY(a >= a);

    FAST_CHECK_UNARY(!(a >= b));
    FAST_CHECK_UNARY(!(a > b));
    FAST_CHECK_UNARY(a < b);
    FAST_CHECK_UNARY(a <= b);
    FAST_CHECK_UNARY(!(a == b));
    FAST_CHECK_UNARY(a != b);

    FAST_CHECK_UNARY(a >= c);
    FAST_CHECK_UNARY(a > c);
    FAST_CHECK_UNARY(!(a < c));
    FAST_CHECK_UNARY(!(a <= c));
    FAST_CHECK_UNARY(!(a == c));
    FAST_CHECK_UNARY(a != c);

    FAST_CHECK_UNARY(b >= c);
    FAST_CHECK_UNARY(b > c);
    FAST_CHECK_UNARY(!(b < c));
    FAST_CHECK_UNARY(!(b <= c));
    FAST_CHECK_UNARY(!(b == c));
    FAST_CHECK_UNARY(b != c);
}

TEST_CASE("money/diff/1") {
    FAST_CHECK_EQ(budget::money(1, 1) - budget::money(1, 1), budget::money(0, 0));
    FAST_CHECK_EQ(budget::money(10, 1) - budget::money(1, 1), budget::money(9, 0));
    FAST_CHECK_EQ(budget::money(10, 1) - budget::money(10, 0), budget::money(0, 1));
    FAST_CHECK_EQ(budget::money(10, 0) - budget::money(100, 0), budget::money(-90, 0));
    FAST_CHECK_EQ(budget::money(10, 0) - budget::money(100, 1), budget::money(-90, 1));
}
