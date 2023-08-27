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
    size_t id;
    std::string guid;
    budget::date date;
    std::string name;
    money amount;
    bool paid;
    money paid_amount;
    size_t importance;
    size_t urgency;

    std::map<std::string, std::string> get_params() const ;

    void load(data_reader & reader);
    void save(data_writer & writer) const;
};

void load_wishes();
void save_wishes();

std::vector<wish> all_wishes();

void set_wishes_changed();
void set_wishes_next_id(size_t next_id);

void list_wishes(budget::writer& w);
void status_wishes(budget::writer& w);
void estimate_wishes(budget::writer& w);

size_t add_wish(wish&& wish);
bool edit_wish(const wish& wish);
bool wish_exists(size_t id);
void wish_delete(size_t id);
wish wish_get(size_t id);

} //end of namespace budget
