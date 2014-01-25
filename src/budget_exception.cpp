//=======================================================================
// Copyright (c) 2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
