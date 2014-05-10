//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef VERSIONING_H
#define VERSIONING_H

#include <vector>
#include <string>

#include "module_traits.hpp"

namespace budget {

struct versioning_module {
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<versioning_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "versioning";
    static constexpr const bool disable_preloading = true;
};

} //end of namespace budget

#endif
