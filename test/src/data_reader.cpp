//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "test.hpp"
#include "data.hpp"
#include "date.hpp"
#include "money.hpp"

using namespace std::string_literals;

TEST_CASE("data_reader/numbers/size_t") {
    budget::data_reader reader;
    reader.parse("99:1,234,567,890:astring:999:-999:99.99:H");

    size_t a, b, c, d,e, f;

    reader >> a;
    reader >> b;
    reader.skip();
    reader >> c;

    FAST_CHECK_EQ(a, 99);
    FAST_CHECK_EQ(b, 1234567890);
    FAST_CHECK_EQ(c, 999);
    FAST_CHECK_EQ(c, 999);
    REQUIRE_THROWS_AS(reader >> d, budget::budget_exception);
    reader.skip();
    REQUIRE_THROWS_AS(reader >> e, budget::budget_exception);
    reader.skip();
    REQUIRE_THROWS_AS(reader >> f, budget::budget_exception);
}

TEST_CASE("data_reader/numbers/int64_t") {
    budget::data_reader reader;
    reader.parse("99:1,234,567,890:astring:999:-999:99.99:H");

    int64_t a, b, c, d, e, f;

    reader >> a;
    reader >> b;
    reader.skip();
    reader >> c;
    reader >> d;

    FAST_CHECK_EQ(a, 99);
    FAST_CHECK_EQ(b, 1234567890);
    FAST_CHECK_EQ(c, 999);
    FAST_CHECK_EQ(c, 999);
    FAST_CHECK_EQ(d, -999);
    REQUIRE_THROWS_AS(reader >> e, budget::budget_exception);
    reader.skip();
    REQUIRE_THROWS_AS(reader >> f, budget::budget_exception);
}

TEST_CASE("data_reader/numbers/int32_t") {
    budget::data_reader reader;
    reader.parse("99:1,234,567,890:astring:999:-999:99.99:H");

    int32_t a, b, c, d, e, f;

    reader >> a;
    reader >> b;
    reader.skip();
    reader >> c;
    reader >> d;

    FAST_CHECK_EQ(a, 99);
    FAST_CHECK_EQ(b, 1234567890);
    FAST_CHECK_EQ(c, 999);
    FAST_CHECK_EQ(c, 999);
    FAST_CHECK_EQ(d, -999);
    REQUIRE_THROWS_AS(reader >> e, budget::budget_exception);
    reader.skip();
    REQUIRE_THROWS_AS(reader >> f, budget::budget_exception);
}

TEST_CASE("data_reader/numbers/bool") {
    budget::data_reader reader;
    reader.parse("0:999:a:123456789:1:a:2020-10-10");

    bool a, b, c, d;

    reader >> a;
    reader >> b;
    reader.skip();
    reader >> c;
    reader >> d;

    FAST_CHECK_UNARY(!a);
    FAST_CHECK_UNARY(b);
    FAST_CHECK_UNARY(c);
    FAST_CHECK_UNARY(d);

    REQUIRE_THROWS_AS(reader >> d, budget::budget_exception);
    reader.skip();
    REQUIRE_THROWS_AS(reader >> d, budget::budget_exception);
}

TEST_CASE("data_reader/date") {
    budget::data_reader reader;
    reader.parse("2020-01-01:2020-12-31:2020-12-01:2020-01-12:1999-03-03:a:2020-100-1");

    budget::date a;
    budget::date b;
    budget::date c;
    budget::date d;
    budget::date e;

    reader >> a;
    reader >> b;
    reader >> c;
    reader >> d;
    reader >> e;

    FAST_CHECK_EQ(a, budget::date(2020, 1, 1));
    FAST_CHECK_EQ(b, budget::date(2020, 12, 31));
    FAST_CHECK_EQ(c, budget::date(2020, 12, 1));
    FAST_CHECK_EQ(d, budget::date(2020, 1, 12));
    FAST_CHECK_EQ(e, budget::date(1999, 3, 3));

    REQUIRE_THROWS_AS(reader >> d, budget::date_exception);
    reader.skip();
    REQUIRE_THROWS_AS(reader >> d, budget::date_exception);
}

TEST_CASE("data_reader/money") {
    budget::data_reader reader;
    reader.parse("100:200.50:0.4:-100.40:-100:a:100.287");

    budget::money a;
    budget::money b;
    budget::money c;
    budget::money d;
    budget::money e;

    reader >> a;
    reader >> b;
    reader >> c;
    reader >> d;
    reader >> e;

    FAST_CHECK_EQ(a, budget::money(100, 0));
    FAST_CHECK_EQ(b, budget::money(200, 50));
    FAST_CHECK_EQ(c, budget::money(0, 4));
    FAST_CHECK_EQ(d, budget::money(-100, 40));
    FAST_CHECK_EQ(e, budget::money(-100, 0));

    REQUIRE_THROWS_AS(reader >> d, budget::budget_exception);
    reader.skip();
    REQUIRE_THROWS_AS(reader >> d, budget::budget_exception);
}
