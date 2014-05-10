//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef BUDGET_EXCEPTION_H
#define BUDGET_EXCEPTION_H

#include <string>

namespace budget {

class budget_exception: public std::exception {
    protected:
        std::string m_message;

    public:
        budget_exception(std::string message);

        ~budget_exception() throw();

        /*!
         * Return the error message.
         * \return The error message.
         */
        const std::string& message() const;

        virtual const char* what() const throw();
};

} //end of budget

#endif
