//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OBJECTIVES_H
#define OBJECTIVES_H

#include <vector>
#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "module_traits.hpp"
#include "money.hpp"

namespace budget {

struct objectives_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<objectives_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "objective";
};

struct objective {
    std::size_t id;
    std::string guid;
    boost::gregorian::date date;
    std::string name;
    std::string type;
    std::string source;
    std::string op;
    money amount;
};

std::ostream& operator<<(std::ostream& stream, const objective& expense);
void operator>>(const std::vector<std::string>& parts, objective& expense);

void load_objectives();
void save_objectives();

std::vector<objective>& all_objectives();
void add_objective(objective&& objective);

void set_objectives_changed();

} //end of namespace budget

#endif
