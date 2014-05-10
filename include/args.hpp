//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ARGS_H
#define ARGS_H

#include <vector>
#include <string>

namespace budget {

std::vector<std::string> parse_args(int argc, const char* argv[]);
void enough_args(const std::vector<std::string>& args, std::size_t min);

} //end of namespace budget

#endif
