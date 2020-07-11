//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <vector>
#include <string>

namespace budget {

std::vector<std::string> parse_args(int argc, const char* argv[], const std::vector<std::pair<const char*, const char*>>& aliases);
void enough_args(const std::vector<std::string>& args, size_t min);

} //end of namespace budget
