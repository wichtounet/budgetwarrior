//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
    static constexpr const bool needs_loading = false;
};

} //end of namespace budget

#endif
