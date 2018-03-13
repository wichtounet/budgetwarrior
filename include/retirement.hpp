//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <vector>
#include <string>

#include "module_traits.hpp"

namespace budget {

struct retirement_module {
    void load();
    void handle(std::vector<std::string>& args);
};

template<>
struct module_traits<retirement_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command   = "retirement";
};

} //end of namespace budget
