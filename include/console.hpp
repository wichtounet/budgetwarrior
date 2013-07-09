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

std::wstring format_code(int attr, int fg, int bg);
void display_table(std::vector<std::wstring> columns, std::vector<std::vector<std::wstring>> contents, std::size_t groups = 1);

} //end of namespace budget

#endif
