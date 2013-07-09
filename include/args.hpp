//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ARGS_H
#define ARGS_H

#include <vector>
#include <string>

namespace budget {

std::vector<std::wstring> parse_args(int argc, const char* argv[]);
void enough_args(const std::vector<std::wstring>& args, std::size_t min);

} //end of namespace budget

#endif
