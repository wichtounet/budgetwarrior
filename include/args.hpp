//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ARGS_H
#define ARGS_H

#include <vector>
#include <string>

namespace budget {

std::vector<std::string> parse_args(int argc, const char* argv[]);

} //end of namespace budget

#endif
