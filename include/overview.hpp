//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
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
#include "writer.hpp"

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

void display_local_balance(budget::writer& , budget::year year, bool current = true, bool relaxed = false, bool last = false);
void display_balance(budget::writer& , budget::year year, bool relaxed = false, bool last = false);
void display_expenses(budget::writer& , budget::year year, bool current = true, bool relaxed = false, bool last = false);
void display_earnings(budget::writer& , budget::year year, bool current = true, bool relaxed = false, bool last = false);

void display_month_overview(budget::month month, budget::year year, budget::writer& );
void display_month_overview(budget::month month, budget::writer& );
void display_month_overview(budget::writer& );

void display_year_overview(budget::year year, budget::writer& w);
void display_year_overview(budget::writer& w);

void aggregate_year_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year);
void aggregate_month_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::month month, budget::year year);

} //end of namespace budget

#endif
