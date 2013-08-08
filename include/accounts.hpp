//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "money.hpp"

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
    std::size_t id;
    std::string guid;
    std::string name;
    money amount;
    boost::gregorian::date since;
    boost::gregorian::date until;
};

std::ostream& operator<<(std::ostream& stream, const account& account);
void operator>>(const std::vector<std::string>& parts, account& account);

void load_accounts();
void save_accounts();

bool account_exists(const std::string& account);

budget::account& get_account(std::size_t id);
budget::account& get_account(std::string name);

std::vector<account>& all_accounts();
std::vector<account>  all_accounts(boost::gregorian::greg_year year, boost::gregorian::greg_month month);

} //end of namespace budget

#endif
