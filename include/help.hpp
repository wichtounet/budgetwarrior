//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef HELP_H
#define HELP_H

#include <vector>
#include <string>

#include "module_traits.hpp"

namespace budget {

struct help_module {
     void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<help_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "help";
};

} //end of namespace budget

#endif
