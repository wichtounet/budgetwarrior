//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef REPORT_H
#define REPORT_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "writer.hpp"

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

void report(budget::writer& w, budget::year year, bool filter, const std::string& filter_account);

} //end of namespace budget

#endif
