//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <vector>
#include <string>

#include "money.hpp"

namespace budget {

struct account {
    std::size_t id;
    std::string guid;
    std::string name;
    money amount;
};

std::ostream& operator<<(std::ostream& stream, const account& account);
void operator>>(const std::vector<std::string>& parts, account& account);

void load_accounts();
void save_accounts();
void add_account(account&& account);
void show_accounts();

int handle_accounts(const std::vector<std::string>& args);

} //end of namespace budget

#endif
