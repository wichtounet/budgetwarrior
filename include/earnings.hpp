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

struct earnings_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<earnings_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "earning";
};

struct earning {
    size_t id;
    std::string guid;
    budget::date date;
    std::string name;
    size_t account;
    money amount;

    std::map<std::string, std::string> get_params() const ;
};

std::ostream& operator<<(std::ostream& stream, const earning& earning);
void operator>>(const std::vector<std::string>& parts, earning& earning);

void load_earnings();
void save_earnings();

std::vector<earning> all_earnings();
void add_earning(earning&& earning);
bool edit_earning(const earning& earning);

void set_earnings_changed();

bool earning_exists(size_t id);
void earning_delete(size_t id);
earning earning_get(size_t id);

void show_all_earnings(budget::writer& w);
void show_earnings(budget::month month, budget::year year, budget::writer& w);
void show_earnings(budget::month month, budget::writer& w);
void show_earnings(budget::writer& w);
void search_earnings(const std::string& search, budget::writer& w);

bool indirect_edit_earning(const earning & earning, bool propagate = true);

// Filter functions

struct data_cache;

filter_view<earning> all_earnings_year(data_cache & cache, budget::year year);
filter_view<earning> all_earnings_month(data_cache & cache, budget::year year, budget::month month);
filter_view<earning> all_earnings_month(data_cache & cache, size_t account_id, budget::year year, budget::month month);
filter_view<earning> all_earnings_between(data_cache & cache, budget::year year, budget::month sm, budget::month month);

} //end of namespace budget
