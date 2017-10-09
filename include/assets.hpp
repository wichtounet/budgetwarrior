//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ASSETS_H
#define ASSETS_H

#include <vector>
#include <string>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"

namespace budget {

struct assets_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<assets_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "asset";
};

struct asset {
    size_t id;
    std::string guid;
    std::string name;
    size_t int_stocks;
    size_t swiss_stocks;
    size_t bonds;
    size_t cash;
    std::string currency;
};

std::ostream& operator<<(std::ostream& stream, const asset& asset);
void operator>>(const std::vector<std::string>& parts, asset& asset);

void load_assets();
void save_assets();

bool asset_exists(const std::string& asset);

budget::asset& get_asset(size_t id);
budget::asset& get_asset(std::string name);

std::vector<budget::asset>& all_assets();

void set_assets_changed();
void set_assets_next_id(size_t next_id);

} //end of namespace budget

#endif
