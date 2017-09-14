//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"

namespace budget {

struct accounts_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<accounts_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "account";
};

struct account {
    size_t id;
    std::string guid;
    std::string name;
    money amount;
    date since;
    date until;
};

std::ostream& operator<<(std::ostream& stream, const account& account);
void operator>>(const std::vector<std::string>& parts, account& account);

void load_accounts();
void save_accounts();

bool account_exists(const std::string& account);

std::vector<budget::account>& all_accounts();
std::vector<budget::account>  all_accounts(year year, month month);

budget::account& get_account(size_t id);
budget::account& get_account(std::string name, year year, month month);

void set_accounts_changed();
void set_accounts_next_id(size_t next_id);

} //end of namespace budget

#endif
