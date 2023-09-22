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

struct recurring_module {
    void load();
    void unload();
    void preload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<recurring_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "recurring";
};

struct recurring {
    size_t      id;
    std::string guid;
    std::string name;
    money       amount;
    std::string recurs;
    std::string account;
    std::string type;

    std::map<std::string, std::string, std::less<>> get_params() const ;

    void load(data_reader & reader);
    void save(data_writer & writer) const;
};

void check_for_recurrings();

void load_recurrings();
void save_recurrings();

std::vector<recurring> all_recurrings();

void set_recurrings_changed();
void set_recurrings_next_id(size_t next_id);

void show_recurrings(budget::writer& w);

size_t add_recurring(recurring&& recurring);
bool edit_recurring(const recurring& recurring, const budget::recurring & previous_recurring);
bool recurring_exists(size_t id);
void recurring_delete(size_t id);
recurring recurring_get(size_t id);

} //end of namespace budget
