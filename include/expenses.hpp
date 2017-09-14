//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef EXPENSES_H
#define EXPENSES_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"

namespace budget {

const date TEMPLATE_DATE(1666, 6, 6);

struct expenses_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<expenses_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "expense";
};

struct expense {
    size_t id;
    std::string guid;
    budget::date date;
    std::string name;
    size_t account;
    money amount;
};

std::ostream& operator<<(std::ostream& stream, const expense& expense);
void operator>>(const std::vector<std::string>& parts, expense& expense);

void load_expenses();
void save_expenses();

std::vector<expense>& all_expenses();
void add_expense(expense&& expense);

void set_expenses_changed();
void set_expenses_next_id(size_t next_id);

} //end of namespace budget

#endif
