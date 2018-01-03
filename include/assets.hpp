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
#include <map>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"
#include "writer_fwd.hpp"

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
    size_t dom_stocks;
    size_t bonds;
    size_t cash;
    std::string currency;
    bool portfolio;
    size_t portfolio_alloc;

    std::map<std::string, std::string> get_params();

    size_t total_allocation() const {
        return int_stocks + dom_stocks + bonds + cash;
    }
};

struct asset_value {
    size_t id;
    std::string guid;
    size_t asset_id;
    budget::money amount;
    budget::date set_date;

    std::map<std::string, std::string> get_params();
};

std::ostream& operator<<(std::ostream& stream, const asset& asset);
void operator>>(const std::vector<std::string>& parts, asset& asset);

std::ostream& operator<<(std::ostream& stream, const asset_value& asset);
void operator>>(const std::vector<std::string>& parts, asset_value& asset);

void load_assets();
void save_assets();

void show_assets(budget::writer& w);
void list_asset_values(budget::writer& w);
void show_asset_values(budget::writer& w);
void show_asset_portfolio(budget::writer& w);
void show_asset_rebalance(budget::writer& w);

bool asset_exists(const std::string& asset);

budget::asset& get_asset(size_t id);
budget::asset& get_asset(std::string name);

budget::asset& get_desired_allocation();

budget::asset_value& get_asset_value(size_t id);

std::vector<budget::asset>& all_assets();
std::vector<budget::asset_value>& all_asset_values();

void set_assets_next_id(size_t next_id);
void set_asset_values_next_id(size_t next_id);

void set_assets_changed();
void set_asset_values_changed();

std::string get_default_currency();

void add_asset(asset&& asset);
bool asset_exists(size_t id);
void asset_delete(size_t id);
asset& asset_get(size_t id);

void add_asset_value(asset_value&& asset_value);
bool asset_value_exists(size_t id);
void asset_value_delete(size_t id);
asset_value& asset_value_get(size_t id);

} //end of namespace budget

#endif
