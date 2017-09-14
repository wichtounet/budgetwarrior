//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef DEBTS_H
#define DEBTS_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"

namespace budget {

struct debt_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<debt_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "debt";
};

struct debt {
    size_t id;
    int state;
    std::string guid;
    budget::date creation_date;
    bool direction;
    std::string name;
    money amount;
    std::string title = "";
};

std::ostream& operator<<(std::ostream& stream, const debt& debt);
void operator>>(const std::vector<std::string>& parts, debt& debt);

void load_debts();
void save_debts();

void migrate_debts_3_to_4();

std::vector<debt>& all_debts();

void set_debts_changed();
void set_debts_next_id(size_t next_id);

} //end of namespace budget

#endif
