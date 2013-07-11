//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "budget_exception.hpp"

budget::budget_exception::budget_exception(std::wstring message) : m_message(std::move(message)) {}

const std::wstring& budget::budget_exception::message() const {
    return m_message;
}
