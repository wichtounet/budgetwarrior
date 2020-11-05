//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "test.hpp"
#include "data.hpp"

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
