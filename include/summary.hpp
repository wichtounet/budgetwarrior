//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef SUMMARY_H
#define SUMMARY_H

#include <vector>
#include <string>
#include <array>

#include "module_traits.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "date.hpp"

namespace budget {

struct summary_module {
    void load();
    void handle(std::vector<std::string>& args);
};

template<>
struct module_traits<summary_module> {
    static constexpr const bool is_default = true;
    static constexpr const char* command = "summary";

    static constexpr const std::array<std::pair<const char*, const char*>, 1> aliases = {{{"aggregate", "overview aggregate"}}};
};

} //end of namespace budget

#endif
