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
#include "filter_iterator.hpp"

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

    std::map<std::string, std::string> get_params() const ;
};

std::ostream& operator<<(std::ostream& stream, const expense& expense);
void operator>>(const std::vector<std::string>& parts, expense& expense);

void load_expenses();
void save_expenses();

std::vector<expense> all_expenses();
void add_expense(expense&& expense);
bool edit_expense(const expense& expense);

void set_expenses_changed();

bool expense_exists(size_t id);
void expense_delete(size_t id);
expense expense_get(size_t id);

void show_all_expenses(budget::writer& w);
void show_expenses(budget::month month, budget::year year, budget::writer& w);
void show_expenses(budget::month month, budget::writer& w);
void show_expenses(budget::writer& w);
void search_expenses(const std::string& search, budget::writer& w);

bool indirect_edit_expense(const expense & expense, bool propagate = true);

// Filter functions

struct data_cache;

filter_view<expense> all_expenses_year(data_cache & cache, budget::year year);
filter_view<expense> all_expenses_month(data_cache & cache, budget::year year, budget::month month);
filter_view<expense> all_expenses_month(data_cache & cache, size_t account_id, budget::year year, budget::month month);
filter_view<expense> all_expenses_month(data_cache & cache, const std::string & account_name, budget::year year, budget::month month);
filter_view<expense> all_expenses_between(data_cache & cache, budget::year year, budget::month sm, budget::month month);
filter_view<expense> all_expenses_between(data_cache & cache, const std::string & account_name, budget::year year, budget::month sm, budget::month month);

} //end of namespace budget
