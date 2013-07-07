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
