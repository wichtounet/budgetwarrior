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

TEST_CASE("date/date_from_string/1") {
    auto as = budget::date_from_string("1988-04-09");
    auto bs = budget::date_from_string("2111-10-10");

    budget::date a(1988, 4, 9);
    budget::date b(2111, 10, 10);

    FAST_CHECK_EQ(a, as);
    FAST_CHECK_EQ(b, bs);
}

TEST_CASE("date/date_from_string/1") {
    // Size must be 10 exactly
    REQUIRE_THROWS_AS(budget::date_from_string("1988-4-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::date_from_string("1988-04-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::date_from_string("88-04-09"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::date_from_string("1988 -04-9"), budget::date_exception);
}

TEST_CASE("date/date_from_string/2") {
    // Each of the parts must be exactly a number
    REQUIRE_THROWS_AS(budget::date_from_string("abcd-4-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::date_from_string("1988-AB-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::date_from_string("88-04-9a"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::date_from_string("1988--4-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::date_from_string("19o8--4-9"), budget::date_exception);
    REQUIRE_THROWS_AS(budget::date_from_string("198 -04-09"), budget::date_exception);
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

TEST_CASE("date/start_of_week") {
    FAST_CHECK_EQ(budget::date(2020, 11, 4).start_of_week(), budget::date(2020, 11, 3));
    FAST_CHECK_EQ(budget::date(2020, 11, 5).start_of_week(), budget::date(2020, 11, 3));
    FAST_CHECK_EQ(budget::date(2020, 11, 3).start_of_week(), budget::date(2020, 11, 3));
    FAST_CHECK_EQ(budget::date(2020, 10, 1).start_of_week(), budget::date(2020, 9, 29));
    FAST_CHECK_EQ(budget::date(2020, 10, 2).start_of_week(), budget::date(2020, 9, 29));
}

TEST_CASE("date/week") {
    FAST_CHECK_EQ(budget::date(2020, 1, 1).week(), 1);
    FAST_CHECK_EQ(budget::date(2020, 11, 4).week(), 45);
    FAST_CHECK_EQ(budget::date(2020, 11, 5).week(), 45);
    FAST_CHECK_EQ(budget::date(2020, 11, 3).week(), 45);
    FAST_CHECK_EQ(budget::date(2020, 10, 1).week(), 40);
    FAST_CHECK_EQ(budget::date(2020, 10, 2).week(), 40);

    FAST_CHECK_EQ(budget::date(2019, 1, 1).week(), 1);
    FAST_CHECK_EQ(budget::date(2019, 11, 4).week(), 45);
    FAST_CHECK_EQ(budget::date(2019, 11, 5).week(), 45);
    FAST_CHECK_EQ(budget::date(2019, 11, 3).week(), 44);
    FAST_CHECK_EQ(budget::date(2019, 10, 1).week(), 40);
    FAST_CHECK_EQ(budget::date(2019, 10, 2).week(), 40);
}

TEST_CASE("date/iso_week") {
    FAST_CHECK_EQ(budget::date(2018, 1, 1).iso_week(), 1);
    FAST_CHECK_EQ(budget::date(2020, 1, 1).iso_week(), 1);

    FAST_CHECK_EQ(budget::date(2020, 11, 1).iso_week(), 44);
    FAST_CHECK_EQ(budget::date(2020, 11, 2).iso_week(), 45);
    FAST_CHECK_EQ(budget::date(2020, 11, 30).iso_week(), 49);
    FAST_CHECK_EQ(budget::date(2020, 12, 1).iso_week(), 49);
    FAST_CHECK_EQ(budget::date(2020, 12, 30).iso_week(), 53);

    FAST_CHECK_EQ(budget::date(2019, 12, 30).iso_week(), 53);
    FAST_CHECK_EQ(budget::date(2019, 12, 31).iso_week(), 53);
}

TEST_CASE("date/iso_start_of_week") {
    FAST_CHECK_EQ(budget::date(2020, 1, 1).iso_start_of_week(), budget::date(2019, 12, 30));
    FAST_CHECK_EQ(budget::date(2018, 1, 1).iso_start_of_week(), budget::date(2018, 1, 1));

    FAST_CHECK_EQ(budget::date(2020, 11, 1).iso_start_of_week(), budget::date(2020, 10, 26));
    FAST_CHECK_EQ(budget::date(2020, 11, 2).iso_start_of_week(), budget::date(2020, 11, 2));
    FAST_CHECK_EQ(budget::date(2020, 11, 3).iso_start_of_week(), budget::date(2020, 11, 2));
    FAST_CHECK_EQ(budget::date(2020, 11, 30).iso_start_of_week(), budget::date(2020, 11, 30));
    FAST_CHECK_EQ(budget::date(2020, 12, 1).iso_start_of_week(), budget::date(2020, 11, 30));
    FAST_CHECK_EQ(budget::date(2020, 12, 1).iso_start_of_week(), budget::date(2020, 11, 30));

    FAST_CHECK_EQ(budget::date(2017, 12, 31).iso_start_of_week(), budget::date(2017, 12, 25));
}

TEST_CASE("date/start_of_month") {
    FAST_CHECK_EQ(budget::date(2020, 11, 4).start_of_month(), budget::date(2020, 11, 1));
    FAST_CHECK_EQ(budget::date(2020, 11, 5).start_of_month(), budget::date(2020, 11, 1));
    FAST_CHECK_EQ(budget::date(2020, 11, 30).start_of_month(), budget::date(2020, 11, 1));
    FAST_CHECK_EQ(budget::date(2020, 10, 1).start_of_month(), budget::date(2020, 10, 1));
    FAST_CHECK_EQ(budget::date(2020, 10, 2).start_of_month(), budget::date(2020, 10, 1));
}

TEST_CASE("date/end_of_month") {
    FAST_CHECK_EQ(budget::date(2020, 11, 4).end_of_month(), budget::date(2020, 11, 30));
    FAST_CHECK_EQ(budget::date(2020, 11, 5).end_of_month(), budget::date(2020, 11, 30));
    FAST_CHECK_EQ(budget::date(2020, 11, 30).end_of_month(), budget::date(2020, 11, 30));
    FAST_CHECK_EQ(budget::date(2020, 10, 1).end_of_month(), budget::date(2020, 10, 31));
    FAST_CHECK_EQ(budget::date(2020, 10, 2).end_of_month(), budget::date(2020, 10, 31));
}

TEST_CASE("date/day_of_year") {
    // Leap year
    FAST_CHECK_EQ(budget::date(2020, 1, 1).day_of_year(), 1);
    FAST_CHECK_EQ(budget::date(2020, 2, 29).day_of_year(), 60);
    FAST_CHECK_EQ(budget::date(2020, 3, 3).day_of_year(), 63);
    FAST_CHECK_EQ(budget::date(2020, 12, 31).day_of_year(), 366);

    // Non-leap year
    FAST_CHECK_EQ(budget::date(2019, 1, 1).day_of_year(), 1);
    FAST_CHECK_EQ(budget::date(2019, 2, 28).day_of_year(), 59);
    FAST_CHECK_EQ(budget::date(2019, 3, 3).day_of_year(), 62);
    FAST_CHECK_EQ(budget::date(2019, 12, 31).day_of_year(), 365);
}

TEST_CASE("date/compare") {
    budget::date a1(2019, 12, 31);
    budget::date a2(2019, 12, 31);
    budget::date b(2020, 1, 1);
    budget::date c(2020, 1, 2);

    // operator==
    FAST_CHECK_UNARY(a1 == a1);
    FAST_CHECK_UNARY(a1 == a2);
    FAST_CHECK_UNARY(!(a1 == b));
    FAST_CHECK_UNARY(!(a1 == c));

    // operator!=
    FAST_CHECK_UNARY(!(a1 != a1));
    FAST_CHECK_UNARY(!(a1 != a2));
    FAST_CHECK_UNARY((a1 != b));
    FAST_CHECK_UNARY((a1 != c));

    // operator<=
    FAST_CHECK_UNARY(a1 <= a1);
    FAST_CHECK_UNARY(a1 <= a2);
    FAST_CHECK_UNARY(a1 <= b);
    FAST_CHECK_UNARY(a1 <= c);
    FAST_CHECK_UNARY(b <= c);
    FAST_CHECK_UNARY(!(c <= b));
    FAST_CHECK_UNARY(!(b <= a1));

    // operator<
    FAST_CHECK_UNARY(!(a1 < a1));
    FAST_CHECK_UNARY(!(a1 < a2));
    FAST_CHECK_UNARY(a1 < b);
    FAST_CHECK_UNARY(a1 < c);
    FAST_CHECK_UNARY(b < c);
    FAST_CHECK_UNARY(!(c < b));
    FAST_CHECK_UNARY(!(b < a1));

    // operator>=
    FAST_CHECK_UNARY(a1 >= a1);
    FAST_CHECK_UNARY(a1 >= a2);
    FAST_CHECK_UNARY(!(a1 >= b));
    FAST_CHECK_UNARY(!(a1 >= c));
    FAST_CHECK_UNARY(!(b >= c));
    FAST_CHECK_UNARY((c >= b));
    FAST_CHECK_UNARY((b >= a1));

    // operator>
    FAST_CHECK_UNARY(!(a1 > a1));
    FAST_CHECK_UNARY(!(a1 > a2));
    FAST_CHECK_UNARY(!(a1 > b));
    FAST_CHECK_UNARY(!(a1 > c));
    FAST_CHECK_UNARY(!(b > c));
    FAST_CHECK_UNARY((c > b));
    FAST_CHECK_UNARY((b > a1));
}

TEST_CASE("date/diff") {
    FAST_CHECK_EQ(budget::date(2020, 3, 3) - budget::date(2020, 3, 3), 0);
    FAST_CHECK_EQ(budget::date(2020, 3, 4) - budget::date(2020, 3, 3), 1);
    FAST_CHECK_EQ(budget::date(2020, 3, 3) - budget::date(2020, 3, 4), -1);
    FAST_CHECK_EQ(budget::date(2020, 4, 4) - budget::date(2020, 3, 3), 32);
    FAST_CHECK_EQ(budget::date(2021, 4, 4) - budget::date(2020, 3, 3), 397);
    FAST_CHECK_EQ(budget::date(2031, 4, 4) - budget::date(2020, 3, 3), 4049);
    FAST_CHECK_EQ(budget::date(2131, 5, 5) - budget::date(2020, 3, 3), 40604);
    FAST_CHECK_EQ(budget::date(2231, 5, 5) - budget::date(2020, 3, 3), 77128);
    FAST_CHECK_EQ(budget::date(2020, 3, 3) - budget::date(2231, 5, 5), -77128);
}
