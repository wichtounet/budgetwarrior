//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef WISHES_H
#define WISHES_H

#include <vector>
#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "module_traits.hpp"
#include "money.hpp"

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
    boost::gregorian::date date;
    std::string name;
    money amount;
};

std::ostream& operator<<(std::ostream& stream, const wish& expense);
void operator>>(const std::vector<std::string>& parts, wish& expense);

void load_wishes();
void save_wishes();

std::vector<wish>& all_wishes();
void add_wish(wish&& objective);

void set_wishes_changed();

} //end of namespace budget

#endif
