//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONSOLE_H
#define CONSOLE_H

#include <vector>
#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "money.hpp"

namespace budget {

/**
 * Returns the real size of a string. By default, accented characteres are
 * represented by several chars and make the length of the string being bigger
 * than its displayable length. This functionr returns only a size of 1 for an
 * accented chars.
 * \param value The string we want the real length for.
 * \return The real length of the string.
 */
std::size_t rsize(const std::string& value);

std::string format_code(int attr, int fg, int bg);
std::string format(const std::string& value);
void display_table(std::vector<std::string> columns, std::vector<std::vector<std::string>> contents, std::size_t groups = 1);

void edit_string(std::string& ref, const std::string& title);
void edit_money(budget::money& ref, const std::string& title);
void edit_date(boost::gregorian::date& ref, const std::string& title);

} //end of namespace budget

#endif
