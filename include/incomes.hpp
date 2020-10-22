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

namespace budget {

struct incomes_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<incomes_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command   = "income";
};

struct income {
    size_t id;
    std::string guid;
    money amount;
    date since;
    date until;

    std::map<std::string, std::string> get_params() const ;
};

std::ostream& operator<<(std::ostream& stream, const income& income);
void operator>>(const std::vector<std::string>& parts, income& income);

void load_incomes();
void save_incomes();

bool income_exists(const std::string& income);

std::vector<budget::income> all_incomes();

void set_incomes_changed();
void set_incomes_next_id(size_t next_id);

void show_all_incomes(budget::writer& w);
void show_incomes(budget::writer& w);

void add_income(income&& income);
bool income_exists(size_t id);
void income_delete(size_t id);
income income_get(size_t id);

void show_incomes(budget::writer& w);

budget::money get_base_income();
budget::money get_base_income(budget::date d);

budget::income new_income(budget::money amount, bool print);

} //end of namespace budget
