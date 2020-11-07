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

TEST_CASE("data_writer/numbers/size_t") {
    budget::data_writer reader;

    size_t a = 0, b = 999, c = 333, d = 1234567890, e = 1, f = 2;

    reader << a;
    reader << b;
    reader << c;
    reader << d;
    reader << e;
    reader << f;

    FAST_CHECK_EQ(reader.to_string(), "0:999:333:1234567890:1:2"s);
}

TEST_CASE("data_writer/numbers/int64_t") {
    budget::data_writer reader;

    int64_t a = 0, b = 999, c = -999, d = 1234567890, e = 1, f = -1;

    reader << a;
    reader << b;
    reader << c;
    reader << d;
    reader << e;
    reader << f;

    FAST_CHECK_EQ(reader.to_string(), "0:999:-999:1234567890:1:-1"s);
}

TEST_CASE("data_writer/numbers/int32_t") {
    budget::data_writer reader;

    int32_t a = 0, b = 999, c = -999, d = 123456789, e = 1, f = -1;

    reader << a;
    reader << b;
    reader << c;
    reader << d;
    reader << e;
    reader << f;

    FAST_CHECK_EQ(reader.to_string(), "0:999:-999:123456789:1:-1"s);
}

TEST_CASE("data_writer/numbers/bool") {
    budget::data_writer reader;

    bool a = 0, b = 999, c = 123456789, d = 1;

    reader << a;
    reader << b;
    reader << c;
    reader << d;

    FAST_CHECK_EQ(reader.to_string(), "0:1:1:1"s);
}

TEST_CASE("data_writer/date") {
    budget::data_writer reader;

    budget::date a(2020, 1, 1);
    budget::date b(2020, 12, 31);
    budget::date c(2020, 12, 1);
    budget::date d(2020, 1, 12);
    budget::date e(1999, 3, 3);

    reader << a;
    reader << b;
    reader << c;
    reader << d;
    reader << e;

    FAST_CHECK_EQ(reader.to_string(), "2020-01-01:2020-12-31:2020-12-01:2020-01-12:1999-03-03"s);
}

TEST_CASE("data_writer/money") {
    budget::data_writer reader;

    budget::money a(0);
    budget::money b(0, 5);
    budget::money c(100, 50);
    budget::money d(-100, 0);
    budget::money e(1,1);

    reader << a;
    reader << b;
    reader << c;
    reader << d;
    reader << e;

    FAST_CHECK_EQ(reader.to_string(), "0.00:0.05:100.50:-100.00:1.01"s);
}
