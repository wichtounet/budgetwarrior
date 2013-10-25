//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef REPORT_H
#define REPORT_H

#include <vector>
#include <string>

#include "module_traits.hpp"

namespace budget {

struct report_module {
    void load();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<report_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "report";
};

} //end of namespace budget

#endif
