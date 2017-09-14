//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef BUDGET_GC_HPP
#define BUDGET_GC_HPP

#include <vector>
#include <string>
#include <array>
#include <utility>

#include "module_traits.hpp"

namespace budget {

struct gc_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<gc_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "gc";
};

} //end of namespace budget

#endif
