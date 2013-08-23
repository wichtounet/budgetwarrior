//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <vector>
#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "module_traits.hpp"

namespace budget {

struct overview_module {
    void load();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<overview_module> {
    static constexpr const bool is_default = true;
    static constexpr const char* command = "overview";
};

} //end of namespace budget

#endif
