//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef EARNINGS_H
#define EARNINGS_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"

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
};

std::ostream& operator<<(std::ostream& stream, const earning& earning);
void operator>>(const std::vector<std::string>& parts, earning& earning);

void load_earnings();
void save_earnings();

std::vector<earning>& all_earnings();
void add_earning(earning&& earning);

void set_earnings_changed();
void set_earnings_next_id(size_t next_id);

} //end of namespace budget

#endif
