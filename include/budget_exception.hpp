//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <string>

namespace budget {

struct budget_exception : std::exception {
    budget_exception(std::string message, bool should_log = false);

    ~budget_exception() noexcept = default;

    /*!
     * Return the error message.
     * \return The error message.
     */
    const std::string& message() const;

    virtual const char* what() const throw();

    bool should_log() const;

protected:
    std::string message_;
    bool        should_log_;
};

} //end of budget
