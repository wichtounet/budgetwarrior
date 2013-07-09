//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEBTS_H
#define DEBTS_H

#include <vector>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "money.hpp"

namespace budget {

typedef boost::posix_time::ptime time;

struct debt {
    std::size_t id;
    int state;
    std::wstring guid;
    time creation_time;
    bool direction;
    std::wstring name;
    money amount;
    std::wstring title;
};

std::wostream& operator<<(std::wostream& stream, const debt& debt);
void operator>>(const std::vector<std::wstring>& parts, debt& debt);

void load_debts();
void save_debts();
void list_debts();
void all_debts();

void handle_debts(const std::vector<std::wstring>& args);

} //end of namespace budget

#endif
