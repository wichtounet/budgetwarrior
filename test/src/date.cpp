//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "test.hpp"
#include "date.hpp"

using namespace std::string_literals;

TEST_CASE("date/to_string") {
    budget::date a(1988, 4, 9);

    FAST_CHECK_EQ(budget::date_to_string(a), "1988-04-09"s);
    FAST_CHECK_EQ(budget::to_string(a), "1988-04-09"s);

    budget::date b(2111, 10, 10);

    FAST_CHECK_EQ(budget::date_to_string(b), "2111-10-10"s);
    FAST_CHECK_EQ(budget::to_string(b), "2111-10-10"s);
}

TEST_CASE("date/from_string/1") {
    auto as = budget::from_string("1988-04-09");
    auto bs = budget::from_string("2111-10-10");

    budget::date a(1988, 4, 9);
    budget::date b(2111, 10, 10);

    FAST_CHECK_EQ(a, as);
    FAST_CHECK_EQ(b, bs);
}

TEST_CASE("date/from_string/1") {
    // Size must be 10 exactly
    REQUIRE_THROWS_AS(budget::from_string("1988-4-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::from_string("1988-04-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::from_string("88-04-09"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::from_string("1988 -04-9"), budget::date_exception);
}

TEST_CASE("date/from_string/2") {
    // Each of the parts must be exactly a number
    REQUIRE_THROWS_AS(budget::from_string("abcd-4-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::from_string("1988-AB-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::from_string("88-04-9a"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::from_string("1988--4-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::from_string("19o8--4-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::from_string("198 -04-09"), budget::date_exception);
}

TEST_CASE("date/minus/days") {
    budget::date a(2010, 5, 6);
    a -= budget::days(1);

    FAST_CHECK_EQ(a.year(), 2010);
    FAST_CHECK_EQ(a.month(), 5);
    FAST_CHECK_EQ(a.day(), 5);

    a -= budget::days(10);

    FAST_CHECK_EQ(a.year(), 2010);
    FAST_CHECK_EQ(a.month(), 4);
    FAST_CHECK_EQ(a.day(), 25);
}

TEST_CASE("date/minus/month") {
    budget::date a(2010, 5, 6);
    a -= budget::months(1);

    FAST_CHECK_EQ(a.year(), 2010);
    FAST_CHECK_EQ(a.month(), 4);
    FAST_CHECK_EQ(a.day(), 6);

    a -= budget::months(10);

    FAST_CHECK_EQ(a.year(), 2009);
    FAST_CHECK_EQ(a.month(), 6);
    FAST_CHECK_EQ(a.day(), 6);
}

TEST_CASE("date/minus/years") {
    budget::date a(2010, 5, 6);
    a -= budget::years(1);

    FAST_CHECK_EQ(a.year(), 2009);
    FAST_CHECK_EQ(a.month(), 5);
    FAST_CHECK_EQ(a.day(), 6);

    a -= budget::years(10);

    FAST_CHECK_EQ(a.year(), 1999);
    FAST_CHECK_EQ(a.month(), 5);
    FAST_CHECK_EQ(a.day(), 6);
}

TEST_CASE("date/plus/days") {
    budget::date a(2010, 5, 6);
    a += budget::days(1);

    FAST_CHECK_EQ(a.year(), 2010);
    FAST_CHECK_EQ(a.month(), 5);
    FAST_CHECK_EQ(a.day(), 7);

    a += budget::days(30);

    FAST_CHECK_EQ(a.year(), 2010);
    FAST_CHECK_EQ(a.month(), 6);
    FAST_CHECK_EQ(a.day(), 6);
}

TEST_CASE("date/plus/month") {
    budget::date a(2010, 5, 6);
    a += budget::months(1);

    FAST_CHECK_EQ(a.year(), 2010);
    FAST_CHECK_EQ(a.month(), 6);
    FAST_CHECK_EQ(a.day(), 6);

    a += budget::months(10);

    FAST_CHECK_EQ(a.year(), 2011);
    FAST_CHECK_EQ(a.month(), 4);
    FAST_CHECK_EQ(a.day(), 6);
}

TEST_CASE("date/plus/years") {
    budget::date a(2010, 5, 6);
    a += budget::years(1);

    FAST_CHECK_EQ(a.year(), 2011);
    FAST_CHECK_EQ(a.month(), 5);
    FAST_CHECK_EQ(a.day(), 6);

    a += budget::years(10);

    FAST_CHECK_EQ(a.year(), 2021);
    FAST_CHECK_EQ(a.month(), 5);
    FAST_CHECK_EQ(a.day(), 6);
}

TEST_CASE("date/is_leap") {
    FAST_CHECK_UNARY(budget::date(1804, 1, 1).is_leap());
    FAST_CHECK_UNARY(budget::date(1944, 1, 1).is_leap());
    FAST_CHECK_UNARY(budget::date(2000, 1, 1).is_leap());
    FAST_CHECK_UNARY(budget::date(2212, 1, 1).is_leap());
    FAST_CHECK_UNARY(budget::date(2400, 1, 1).is_leap());

    FAST_CHECK_UNARY(!budget::date(1805, 1, 1).is_leap());
    FAST_CHECK_UNARY(!budget::date(1943, 1, 1).is_leap());
    FAST_CHECK_UNARY(!budget::date(2001, 1, 1).is_leap());
    FAST_CHECK_UNARY(!budget::date(2399, 1, 1).is_leap());
}

TEST_CASE("date/day_of_the_week") {
    // A leap year
    FAST_CHECK_EQ(budget::date(2000, 3, 3).day_of_the_week(), 5);
    FAST_CHECK_EQ(budget::date(2000, 3, 4).day_of_the_week(), 6);
    FAST_CHECK_EQ(budget::date(2000, 3, 5).day_of_the_week(), 7);
    FAST_CHECK_EQ(budget::date(2000, 3, 6).day_of_the_week(), 1);

    // A non-leap year
    FAST_CHECK_EQ(budget::date(2019, 3, 3).day_of_the_week(), 7);
    FAST_CHECK_EQ(budget::date(2019, 3, 4).day_of_the_week(), 1);
    FAST_CHECK_EQ(budget::date(2019, 3, 5).day_of_the_week(), 2);
    FAST_CHECK_EQ(budget::date(2019, 3, 6).day_of_the_week(), 3);
    FAST_CHECK_EQ(budget::date(2019, 3, 7).day_of_the_week(), 4);
}

// TODO Test for leap years
// TODO Test for start_of_week
// TODO Test for start_of_month
// TODO Test for end_of_month
// TODO Test for week()
// TODO Test for year_days()
// TODO Test for comparisons
// TODO Test for date differences
