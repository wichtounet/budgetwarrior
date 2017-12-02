//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef OBJECTIVES_H
#define OBJECTIVES_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "money.hpp"
#include "compute.hpp"
#include "date.hpp"
#include "writer.hpp"

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
    size_t id;
    std::string guid;
    budget::date date;
    std::string name;
    std::string type;
    std::string source;
    std::string op;
    money amount;
};

std::ostream& operator<<(std::ostream& stream, const objective& expense);
void operator>>(const std::vector<std::string>& parts, objective& expense);

void yearly_objective_status(budget::writer& w, bool lines, bool full_align);
void monthly_objective_status(budget::writer& w);
void current_monthly_objective_status(budget::writer& w, bool full_align);

void load_objectives();
void save_objectives();

std::vector<objective>& all_objectives();
void add_objective(objective&& objective);

void set_objectives_changed();
void set_objectives_next_id(size_t next_id);

int compute_success(const budget::status& status, const objective& objective);

void list_objectives(budget::writer& w);
void status_objectives(budget::writer& w);

} //end of namespace budget

#endif
