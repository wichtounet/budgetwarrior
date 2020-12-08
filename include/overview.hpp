//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <vector>
#include <string>
#include <array>

#include "module_traits.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "date.hpp"
#include "writer_fwd.hpp"

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
void display_month_account_overview(size_t account_id, budget::month month, budget::year year, budget::writer& );

void display_year_overview_header(budget::year year, budget::writer& w);
void display_year_overview(budget::year year, budget::writer& w);

void aggregate_all_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator);
void aggregate_year_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year);
void aggregate_year_month_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year);
void aggregate_year_fv_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year);
void aggregate_month_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::month month, budget::year year);

// Utilities

void add_expenses_column(budget::month                            month,
                         budget::year                             year,
                         const std::string&                       title,
                         std::vector<std::vector<std::string>>&   contents,
                         std::unordered_map<std::string, size_t>& indexes,
                         size_t                                   columns,
                         const std::vector<expense>&              values,
                         std::vector<budget::money>&              total);

void add_earnings_column(budget::month                            month,
                         budget::year                             year,
                         const std::string&                       title,
                         std::vector<std::vector<std::string>>&   contents,
                         std::unordered_map<std::string, size_t>& indexes,
                         size_t                                   columns,
                         const std::vector<earning>&              values,
                         std::vector<budget::money>&              total);

} //end of namespace budget
