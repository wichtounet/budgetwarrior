//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "writer.hpp"

budget::console_writer::console_writer(std::ostream& os) : os(os) {}

budget::writer& budget::console_writer::operator<<(const std::string& value){
    os << value;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const double& value){
    os << value;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::month& m) {
    os << m.as_short_string();

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::year& y) {
    os << y.value;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::end_of_line_t&) {
    os << std::endl;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::p_begin_t&) {
    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::p_end_t&) {
    os << std::endl;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::title_begin_t&) {
    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::title_end_t&) {
    os << std::endl << std::endl;

    return *this;
}
