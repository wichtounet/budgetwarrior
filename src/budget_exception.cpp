//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "budget_exception.hpp"

budget::budget_exception::budget_exception(std::string message) : m_message(std::move(message)) {}

budget::budget_exception::~budget_exception() throw() {}

const char* budget::budget_exception::what() const throw() {
    return m_message.c_str();
}

const std::string& budget::budget_exception::message() const {
    return m_message;
}
