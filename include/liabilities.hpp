//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <vector>
#include <string>
#include <map>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"
#include "writer_fwd.hpp"
#include "filter_iterator.hpp"

namespace budget {

struct liabilities_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<liabilities_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "liability";
};

// A liability
struct liability {
    size_t id;
    std::string guid;
    std::string name;
    std::string currency;

    std::map<std::string, std::string> get_params() const ;
};

std::ostream& operator<<(std::ostream& stream, const liability& liability);
void operator>>(const std::vector<std::string>& parts, liability& liability);

void load_liabilities();
void save_liabilities();

void show_liabilities(budget::writer& w);

budget::liability& get_liability(size_t id);
budget::liability& get_liability(std::string name);

std::vector<budget::liability> all_liabilities();

budget::date liability_start_date();
budget::date liability_start_date(const liability& liability);

void set_liabilities_next_id(size_t next_id);

void set_liabilities_changed();

void add_liability(liability& liability);
bool liability_exists(size_t id);
bool liability_exists(const std::string& name);
void liability_delete(size_t id);
liability& liability_get(size_t id);

// The value of a liability in its own currency
budget::money get_liability_value(budget::liability & liability);
budget::money get_liability_value(budget::liability & liability, budget::date d);

// The value of a liability in the default currency
budget::money get_liability_value_conv(budget::liability & liability);
budget::money get_liability_value_conv(budget::liability & liability, budget::date d);

// The value of a liability in a specific currency
budget::money get_liability_value_conv(budget::liability & liability, const std::string& currency);
budget::money get_liability_value_conv(budget::liability & liability, budget::date d, const std::string& currency);

} //end of namespace budget
