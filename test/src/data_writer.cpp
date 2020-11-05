//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "test.hpp"
#include "data.hpp"

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
