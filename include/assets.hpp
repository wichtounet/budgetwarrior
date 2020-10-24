//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <vector>
#include <string>
#include <map>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"
#include "writer_fwd.hpp"
#include "filter_iterator.hpp"

namespace budget {

struct data_cache;

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

// An asset class
struct asset_class {
    size_t id;
    std::string guid;
    std::string name;

    std::map<std::string, std::string> get_params() const ;
};

budget::asset_class get_asset_class(size_t id);
budget::asset_class get_asset_class(const std::string & name);

// An asset
struct asset {
    size_t id;
    std::string guid;
    std::string name;
    std::string currency;
    bool portfolio;
    money portfolio_alloc;
    bool share_based;
    std::string ticker;
    std::vector<std::pair<size_t, money>> classes;

    // Legacy fields, to be removed
    money int_stocks;
    money dom_stocks;
    money bonds;
    money cash;

    std::map<std::string, std::string> get_params() const ;

    money total_allocation() const {
        money total;

        for (auto& [class_id, alloc] : classes) {
            total += alloc;
        }

        return total;
    }

    bool is_cash() const {
        for (auto& [class_id, alloc] : classes) {
            if (get_asset_class(class_id).name == "cash") {
                return alloc == budget::money(100);
            }

            if (get_asset_class(class_id).name == "Cash") {
                return alloc == budget::money(100);
            }
        }

        return false;
    }
};

// Used to set the value of the asset
struct asset_value {
    size_t id;
    std::string guid;
    size_t asset_id;
    budget::money amount;
    budget::date set_date;
    bool liability;

    std::map<std::string, std::string> get_params() const ;
};

// Used to indicate purchase of shares
struct asset_share {
    size_t id;
    std::string guid;
    size_t asset_id;
    int64_t shares;      // The number of shares
    budget::money price; // The purchase price
    budget::date date;   // The purchase date

    std::map<std::string, std::string> get_params() const ;

    bool is_buy() const {
        return shares >= 0;
    }

    bool is_sell() const {
        return shares < 0;
    }
};

std::ostream& operator<<(std::ostream& stream, const asset_class& c);
void operator>>(const std::vector<std::string>& parts, asset_class& c);

std::ostream& operator<<(std::ostream& stream, const asset& asset);
void operator>>(const std::vector<std::string>& parts, asset& asset);

std::ostream& operator<<(std::ostream& stream, const asset_value& asset);
void operator>>(const std::vector<std::string>& parts, asset_value& asset);

std::ostream& operator<<(std::ostream& stream, const asset_share& asset);
void operator>>(const std::vector<std::string>& parts, asset_share& asset);

void load_asset_classes();
void save_asset_classes();

void load_asset_values();
void save_asset_values();

void load_asset_shares();
void save_asset_shares();

void load_assets();
void save_assets();

void migrate_assets_4_to_5();
void migrate_assets_5_to_6();

void show_asset_classes(budget::writer& w);

void show_assets(budget::writer& w);
void list_asset_values(budget::writer& w, bool liability = false);
void small_show_asset_values(budget::writer& w);
void show_asset_values(budget::writer& w, bool liability = false);
void show_asset_portfolio(budget::writer& w);
void show_asset_rebalance(budget::writer& w, bool nocash = false);

void list_asset_shares(budget::writer& w);

bool asset_class_exists(const std::string& name);

bool asset_exists(const std::string& asset);
bool share_asset_exists(const std::string& asset);

budget::asset get_asset(size_t id);
budget::asset get_asset(std::string name);

budget::asset_value get_asset_value(size_t id);
budget::asset_share get_asset_share(size_t id);

budget::asset get_desired_allocation();

std::vector<budget::asset_class> all_asset_classes();
std::vector<budget::asset> all_assets();
std::vector<budget::asset_value> all_asset_values();
std::vector<budget::asset_share> all_asset_shares();

budget::date asset_start_date();
budget::date asset_start_date(const asset& asset);

void set_asset_class_next_id(size_t next_id);
void set_assets_next_id(size_t next_id);
void set_asset_values_next_id(size_t next_id);
void set_asset_shares_next_id(size_t next_id);

void set_asset_classes_changed();
void set_assets_changed();
void set_asset_values_changed();
void set_asset_shares_changed();

std::string get_default_currency();

size_t add_asset_class(asset_class& c);
bool edit_asset_class(asset_class& c);
bool asset_class_exists(size_t id);
void asset_class_delete(size_t id);

void add_asset(asset& asset);
bool asset_exists(size_t id);
void asset_delete(size_t id);

size_t add_asset_value(asset_value& asset_value);
bool edit_asset_value(asset_value& asset_value);
bool asset_value_exists(size_t id);
void asset_value_delete(size_t id);

size_t add_asset_share(asset_share& asset_share);
bool edit_asset_share(asset_share& c);
bool asset_share_exists(size_t id);
void asset_share_delete(size_t id);

budget::money get_portfolio_value();
budget::money get_net_worth_cash();

budget::money get_net_worth();
budget::money get_net_worth(data_cache & cache);
budget::money get_net_worth(budget::date d);
budget::money get_net_worth(budget::date d, data_cache & cache);

// The value of an asset in its own currency
budget::money get_asset_value(const budget::asset & asset, data_cache & cache);
budget::money get_asset_value(const budget::asset & asset, budget::date d, data_cache & cache);

// The value of an asset in the default currency
budget::money get_asset_value_conv(const budget::asset & asset, data_cache & cache);
budget::money get_asset_value_conv(const budget::asset & asset, budget::date d, data_cache & cache);

// The value of an asset in a specific currency
budget::money get_asset_value_conv(const budget::asset & asset, const std::string& currency, data_cache & cache);
budget::money get_asset_value_conv(const budget::asset & asset, budget::date d, const std::string& currency, data_cache & cache);

// Utilities for assets
void update_asset_class_allocation(budget::asset& asset, budget::asset_class & clas, budget::money alloc);
budget::money get_asset_class_allocation(const budget::asset& asset, const budget::asset_class & clas);

} //end of namespace budget
