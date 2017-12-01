//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef WRITER_H
#define WRITER_H

#include <iostream>
#include <string>

#include "date.hpp"

namespace budget {

struct end_of_line_t {};
struct title_begin_t {};
struct title_end_t {};
struct p_begin_t {};
struct p_end_t {};

static constexpr end_of_line_t end_of_line;
static constexpr p_begin_t p_begin;
static constexpr p_end_t p_end;
static constexpr title_begin_t title_begin;
static constexpr title_end_t title_end;

struct writer {
    virtual writer& operator<<(const std::string& value) = 0;
    virtual writer& operator<<(const double& value) = 0;

    virtual writer& operator<<(const budget::month& m) = 0;
    virtual writer& operator<<(const budget::year& m) = 0;

    virtual writer& operator<<(const end_of_line_t& m) = 0;
    virtual writer& operator<<(const p_begin_t& m) = 0;
    virtual writer& operator<<(const p_end_t& m) = 0;
    virtual writer& operator<<(const title_begin_t& m) = 0;
    virtual writer& operator<<(const title_end_t& m) = 0;
};

struct console_writer : writer {
    std::ostream& os;

    console_writer(std::ostream& os);

    virtual writer& operator<<(const std::string& value) override;
    virtual writer& operator<<(const double& value) override;

    virtual writer& operator<<(const budget::month& m) override;
    virtual writer& operator<<(const budget::year& m) override;

    virtual writer& operator<<(const end_of_line_t& m) override;
    virtual writer& operator<<(const p_begin_t& m) override;
    virtual writer& operator<<(const p_end_t& m) override;
    virtual writer& operator<<(const title_begin_t& m) override;
    virtual writer& operator<<(const title_end_t& m) override;
};

} //end of namespace budget

#endif
