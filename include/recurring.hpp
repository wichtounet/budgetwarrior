//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef RECURRING_H
#define RECURRING_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "money.hpp"

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
    std::size_t id;
    std::string guid;
    std::string name;
    std::size_t old_account;
    money amount;
    std::string recurs;
    std::string account;
};

std::ostream& operator<<(std::ostream& stream, const recurring& recurring);
void operator>>(const std::vector<std::string>& parts, recurring& recurring);

void load_recurrings();
void save_recurrings();

void migrate_recurring_1_to_2();

std::vector<recurring>& all_recurrings();

} //end of namespace budget

#endif
