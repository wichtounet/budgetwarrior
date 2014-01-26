//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FORTUNE_H
#define FORTUNE_H

#include <vector>
#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "module_traits.hpp"
#include "money.hpp"

namespace budget {

struct fortune_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<fortune_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "fortune";
};

struct fortune {
    std::size_t id;
    std::string guid;
    boost::gregorian::date check_date;
    money amount;
};

std::ostream& operator<<(std::ostream& stream, const fortune& fortune);
void operator>>(const std::vector<std::string>& parts, fortune& fortune);

std::vector<fortune>& all_fortunes();

void load_fortunes();
void save_fortunes();

} //end of namespace budget

#endif
