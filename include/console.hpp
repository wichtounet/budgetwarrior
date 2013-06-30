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

namespace budget {

std::string format_code(int attr, int fg, int bg);
void display_table(std::vector<std::string> columns, std::vector<std::vector<std::string>> contents);

} //end of namespace budget

#endif
