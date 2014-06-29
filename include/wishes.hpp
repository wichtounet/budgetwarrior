//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef WISHES_H
#define WISHES_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"

namespace budget {

struct wishes_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<wishes_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "wish";
};

struct wish {
    std::size_t id;
    std::string guid;
    date date;
    std::string name;
    money amount;
    bool paid;
    money paid_amount;
    std::size_t importance;
    std::size_t urgency;
};

std::ostream& operator<<(std::ostream& stream, const wish& expense);
void operator>>(const std::vector<std::string>& parts, wish& expense);

void load_wishes();
void save_wishes();

std::vector<wish>& all_wishes();
void add_wish(wish&& objective);

void set_wishes_changed();

void migrate_wishes_2_to_3();
void migrate_wishes_3_to_4();

} //end of namespace budget

#endif
