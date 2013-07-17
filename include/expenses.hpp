//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef EXPENSES_H
#define EXPENSES_H

#include <vector>
#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "module_traits.hpp"
#include "money.hpp"

namespace budget {

struct expenses_module {
     void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<expenses_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "expense";
};

struct expense {
    std::size_t id;
    std::string guid;
    boost::gregorian::date date;
    std::string name;
    std::size_t account;
    money amount;
};

std::ostream& operator<<(std::ostream& stream, const expense& expense);
void operator>>(const std::vector<std::string>& parts, expense& expense);

void load_expenses();
void save_expenses();

std::vector<expense>& all_expenses();

} //end of namespace budget

#endif
