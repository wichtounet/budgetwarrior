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

struct data_reader;
struct data_writer;

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

    std::map<std::string, std::string, std::less<>> get_params() const ;

    void load(data_reader & reader);
    void save(data_writer& writer) const;
};

void load_incomes();
void save_incomes();

bool income_exists(const std::string& income);

std::vector<budget::income> all_incomes();

void set_incomes_changed();
void set_incomes_next_id(size_t next_id);

void show_all_incomes(budget::writer& w);
void show_incomes(budget::writer& w);

size_t add_income(income&& income);
bool edit_income(const income& income);
bool income_exists(size_t id);
void income_delete(size_t id);
income income_get(size_t id);

struct data_cache;

budget::money get_base_income(data_cache & cache);
budget::money get_base_income(data_cache& cache, const budget::date& d);

budget::income new_income(budget::money amount, bool print);

} //end of namespace budget
