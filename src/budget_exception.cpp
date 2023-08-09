//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "budget_exception.hpp"

budget::budget_exception::budget_exception(std::string message, bool should_log) : message_(std::move(message)), should_log_(should_log) {}

const char* budget::budget_exception::what() const noexcept { return message_.c_str(); }

const std::string& budget::budget_exception::message() const {
    return message_;
}

bool budget::budget_exception::should_log() const {
    return should_log_;
}
