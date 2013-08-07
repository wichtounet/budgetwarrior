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

#include "module_traits.hpp"
#include "money.hpp"

namespace budget {

struct debt_module {
     void load();
     void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<debt_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "debt";
    static constexpr const bool needs_loading = true;
};

struct debt {
    std::size_t id;
    int state;
    std::string guid;
    boost::posix_time::ptime creation_time;
    bool direction;
    std::string name;
    money amount;
    std::string title = "";
};

std::ostream& operator<<(std::ostream& stream, const debt& debt);
void operator>>(const std::vector<std::string>& parts, debt& debt);

void load_debts();
void save_debts();

} //end of namespace budget

#endif
