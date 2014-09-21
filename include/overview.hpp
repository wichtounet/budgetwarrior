//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <vector>
#include <string>
#include <array>

#include "module_traits.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "date.hpp"

namespace budget {

struct overview_module {
    void load();
    void handle(std::vector<std::string>& args);
};

template<>
struct module_traits<overview_module> {
    static constexpr const bool is_default = true;
    static constexpr const char* command = "overview";

    static constexpr const std::array<std::pair<const char*, const char*>, 1> aliases = {{{"aggregate", "overview aggregate"}}};
};

void display_local_balance(budget::year year, bool current = true, bool relaxed = false);
void display_balance(budget::year year, bool current = true, bool relaxed = false);
void display_expenses(budget::year year, bool current = true, bool relaxed = false);
void display_earnings(budget::year year, bool current = true, bool relaxed = false);

} //end of namespace budget

#endif
