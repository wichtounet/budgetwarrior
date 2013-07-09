//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BUDGET_EXCEPTION_H
#define BUDGET_EXCEPTION_H

#include <string>

namespace budget {

class budget_exception {
    protected:
        std::wstring m_message;

    public:
        budget_exception(std::wstring message);

        /*!
         * Return the error message.
         * \return The error message.
         */
        const std::wstring& message() const;
};

} //end of budget

#endif
