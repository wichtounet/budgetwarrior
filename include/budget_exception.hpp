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
    explicit budget_exception(std::string message, bool should_log = false);

    ~budget_exception() noexcept override = default;

    /*!
     * Return the error message.
     * \return The error message.
     */
    const std::string& message() const;

    const char* what() const noexcept override;

    bool should_log() const;

protected:
    std::string message_;
    bool        should_log_;
};

} //namespace budget
