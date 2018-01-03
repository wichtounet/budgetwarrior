//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef FORTUNE_H
#define FORTUNE_H

#include <vector>
#include <string>
#include <map>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"
#include "writer_fwd.hpp"

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
    size_t id;
    std::string guid;
    date check_date;
    money amount;

    std::map<std::string, std::string> get_params();
};

std::ostream& operator<<(std::ostream& stream, const fortune& fortune);
void operator>>(const std::vector<std::string>& parts, fortune& fortune);

budget::money current_fortune();

void load_fortunes();
void save_fortunes();

std::vector<fortune>& all_fortunes();

void list_fortunes(budget::writer& w);
void status_fortunes(budget::writer& w, bool short_view);

void set_fortunes_changed();
void set_fortunes_next_id(size_t next_id);

void add_fortune(fortune&& fortune);
bool fortune_exists(size_t id);
void fortune_delete(size_t id);
fortune& fortune_get(size_t id);

} //end of namespace budget

#endif
