//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef RECURRING_H
#define RECURRING_H

#include <vector>
#include <string>
#include <map>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"
#include "writer_fwd.hpp"

namespace budget {

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
    size_t id;
    std::string guid;
    std::string name;
    size_t old_account;
    money amount;
    std::string recurs;
    std::string account;

    std::map<std::string, std::string> get_params();
};

std::ostream& operator<<(std::ostream& stream, const recurring& recurring);
void operator>>(const std::vector<std::string>& parts, recurring& recurring);

year first_year(const recurring& recurring);
month first_month(const recurring& recurring, budget::year year);

year last_year(const recurring& recurring);
month last_month(const recurring& recurring, budget::year year);

void load_recurrings();
void save_recurrings();

void migrate_recurring_1_to_2();

std::vector<recurring>& all_recurrings();

void set_recurrings_changed();
void set_recurrings_next_id(size_t next_id);

void show_recurrings(budget::writer& w);

void add_recurring(recurring&& recurring);
bool recurring_exists(size_t id);
void recurring_delete(size_t id);
recurring& recurring_get(size_t id);

} //end of namespace budget

#endif
