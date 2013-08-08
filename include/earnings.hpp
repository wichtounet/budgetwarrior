//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef EARNINGS_H
#define EARNINGS_H

#include <vector>
#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "module_traits.hpp"
#include "money.hpp"

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
    std::size_t id;
    std::string guid;
    boost::gregorian::date date;
    std::string name;
    std::size_t account;
    money amount;
};

std::ostream& operator<<(std::ostream& stream, const earning& earning);
void operator>>(const std::vector<std::string>& parts, earning& earning);

void load_earnings();
void save_earnings();

std::vector<earning>& all_earnings();

} //end of namespace budget

#endif
