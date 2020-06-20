//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <map>
#include <random>

#include "assets.hpp"
#include "budget_exception.hpp"
#include "args.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "earnings.hpp"
#include "expenses.hpp"
#include "writer.hpp"
#include "currency.hpp"
#include "share.hpp"

#include <curl/curl.h>

using namespace budget;

namespace {

static data_handler<asset_class> asset_classes { "asset_classes", "asset_classes.data" };
static data_handler<asset> assets { "assets", "assets.data" };
static data_handler<asset_value> asset_values { "asset_values", "asset_values.data" };
static data_handler<asset_share> asset_shares { "asset_shares", "asset_shares.data" };

std::vector<std::string> get_asset_names(){
    std::vector<std::string> asset_names;

    for (auto& asset : all_user_assets()) {
        asset_names.push_back(asset.name);
    }

    return asset_names;
}

std::vector<std::string> get_share_asset_names(){
    std::vector<std::string> asset_names;

    for (auto& asset : all_user_assets()) {
        if (asset.share_based) {
            asset_names.push_back(asset.name);
        }
    }

    return asset_names;
}

void update_asset_class_allocation(budget::asset& asset, budget::asset_class & clas, budget::money alloc) {
    for (auto & c : asset.classes) {
        if (c.first == clas.id) {
            c.second = alloc;
            return;
        }
    }

    asset.classes.emplace_back(clas.id, alloc);
}

} //end of anonymous namespace

std::map<std::string, std::string> budget::asset_class::get_params(){
    std::map<std::string, std::string> params;

    params["input_id"]              = budget::to_string(id);
    params["input_guid"]            = guid;
    params["input_name"]            = name;

    return params;
}

std::map<std::string, std::string> budget::asset::get_params(){
    std::map<std::string, std::string> params;

    params["input_id"]              = budget::to_string(id);
    params["input_guid"]            = guid;
    params["input_name"]            = name;
    params["input_currency"]        = currency;
    params["input_portfolio"]       = portfolio ? "true" : "false";
    params["input_portfolio_alloc"] = budget::to_string(portfolio_alloc);
    params["input_shared_based"]    = share_based ? "true" : "false";
    params["input_ticker"]          = ticker;

    // The asset classes allocation
    for (auto & clas : classes) {
        params["input_class_" + to_string(clas.first)] = budget::to_string(clas.second);
    }

    return params;
}

std::map<std::string, std::string> budget::asset_value::get_params(){
    std::map<std::string, std::string> params;

    params["input_id"]       = budget::to_string(id);
    params["input_guid"]     = guid;
    params["input_asset_id"] = budget::to_string(asset_id);
    params["input_amount"]   = budget::to_string(amount);
    params["input_set_date"] = budget::to_string(set_date);

    return params;
}

std::map<std::string, std::string> budget::asset_share::get_params(){
    std::map<std::string, std::string> params;

    params["input_id"]       = budget::to_string(id);
    params["input_guid"]     = guid;
    params["input_asset_id"] = budget::to_string(asset_id);
    params["input_price"]    = budget::to_string(price);
    params["input_date"]     = budget::to_string(date);
    params["input_shares"]   = budget::to_string(shares);

    return params;
}

void budget::assets_module::load(){
    load_assets();
}

void budget::assets_module::unload(){
    save_assets();
}

void budget::assets_module::handle(const std::vector<std::string>& args){
    console_writer w(std::cout);

    if(args.size() == 1){
        show_assets(w);
    } else {
        auto& subcommand = args[1];

        if(subcommand == "show"){
            show_assets(w);
        } else if (subcommand == "rebalance") {
            show_asset_rebalance(w);
        } else if (subcommand == "portfolio") {
            budget::show_asset_portfolio(w);
        } else if(subcommand == "add"){
            asset asset;
            asset.guid = generate_guid();

            edit_string(asset.name, "Name", not_empty_checker());

            if(asset_exists(asset.name)){
                throw budget_exception("An asset with this name already exists");
            }

            asset.portfolio       = false;
            asset.portfolio_alloc = 0;
            asset.share_based     = false;
            asset.ticker          = "";

            do {
                for (auto & clas : all_asset_classes()) {
                    budget::money alloc = get_asset_class_allocation(asset, clas);
                    edit_money(alloc, clas.name);
                    update_asset_class_allocation(asset, clas, alloc);
                }

                if (asset.total_allocation() != money(100)) {
                    std::cout << "The distribution must account to 100%" << std::endl;
                }
            } while (asset.total_allocation() != money(100));

            asset.currency = budget::get_default_currency();
            edit_string(asset.currency, "Currency", not_empty_checker());

            std::cout << "Is this part of your portfolio ? [yes/no] ? ";

            std::string answer;

            std::getline(std::cin, answer);
            asset.portfolio = answer == "yes" || answer == "y";

            if (asset.portfolio) {
                edit_money(asset.portfolio_alloc, "Portfolio Allocation");
            }

            std::cout << "Is this asset managed with shares ? [yes/no] ? ";

            std::getline(std::cin, answer);
            asset.share_based = answer == "yes" || answer == "y";

            if (asset.share_based) {
                edit_string(asset.ticker, "Ticker", not_empty_checker());
            }

            auto id = assets.add(std::move(asset));
            std::cout << "Asset " << id << " has been created" << std::endl;
        } else if(subcommand == "delete"){
            size_t id = 0;

            if(args.size() >= 3){
                enough_args(args, 3);

                id = to_number<size_t>(args[2]);

                if (!assets.exists(id)) {
                    throw budget_exception("There are no asset with id " + args[2]);
                }
            } else {
                std::string name;
                edit_string_complete(name, "Asset", get_asset_names(), not_empty_checker(), asset_checker());

                id = budget::get_asset(name).id;
            }

            auto& asset = get_asset(id);

            if(asset.name == "DESIRED" && asset.currency == "DESIRED"){
                throw budget_exception("Cannot delete special asset " + args[2]);
            }

            for (auto& value : asset_values.data) {
                if (value.asset_id == id) {
                    throw budget_exception("There are still asset values linked to asset " + args[2]);
                }
            }

            for (auto& share : asset_shares.data) {
                if (share.asset_id == id) {
                    throw budget_exception("There are still asset shares linked to asset " + args[2]);
                }
            }

            assets.remove(id);

            std::cout << "Asset " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            size_t id = 0;

            if(args.size() >= 3){
                enough_args(args, 3);

                id = to_number<size_t>(args[2]);

                if(!assets.exists(id)){
                    throw budget_exception("There are no asset with id " + args[2]);
                }
            } else {
                std::string name;
                edit_string_complete(name, "Asset", get_asset_names(), not_empty_checker(), asset_checker());

                id = get_asset(name).id;
            }

            auto& asset = assets[id];

            edit_string(asset.name, "Name", not_empty_checker());

            //Verify that there are no OTHER asset with this name
            //in the current set of assets (taking archiving into asset)

            for (auto& other_asset : all_user_assets()) {
                if (other_asset.id != id) {
                    if (other_asset.name == asset.name) {
                        throw budget_exception("There is already an asset with the name " + asset.name);
                    }
                }
            }

            do {
                for (auto & clas : all_asset_classes()) {
                    budget::money alloc = get_asset_class_allocation(asset, clas);
                    edit_money(alloc, clas.name);
                    update_asset_class_allocation(asset, clas, alloc);
                }

                if (asset.total_allocation() != money(100)) {
                    std::cout << "The distribution must account to 100%" << std::endl;
                }
            } while (asset.total_allocation() != money(100));

            edit_string(asset.currency, "Currency", not_empty_checker());

            std::cout << "Is this part of your portfolio ? [yes/no] ? ";

            std::string answer;

            std::getline(std::cin, answer);
            asset.portfolio = answer == "yes" || answer == "y";

            if (asset.portfolio) {
                edit_money(asset.portfolio_alloc, "Portfolio Allocation");
            } else {
                asset.portfolio_alloc = 0;
            }

            std::cout << "Is this asset managed with shares ? [yes/no] ? ";

            std::getline(std::cin, answer);
            asset.share_based = answer == "yes" || answer == "y";

            if (asset.share_based) {
                edit_string(asset.ticker, "Ticker");
            } else {
                asset.portfolio_alloc = 0;
            }

            if (assets.edit(asset)) {
                std::cout << "Asset " << id << " has been modified" << std::endl;
            }
        } else if(subcommand == "value"){
            if(args.size() == 2){
                budget::show_asset_values(w);
            } else {
                auto& subsubcommand = args[2];

                if(subsubcommand == "set"){
                    asset_value asset_value;
                    asset_value.guid = generate_guid();

                    std::string asset_name;
                    edit_string_complete(asset_name, "Asset", get_asset_names(), not_empty_checker(), asset_checker());
                    asset_value.asset_id = get_asset(asset_name).id;

                    edit_money(asset_value.amount, "Amount", not_negative_checker());

                    asset_value.set_date = budget::local_day();
                    edit_date(asset_value.set_date, "Date");

                    auto id = asset_values.add(std::move(asset_value));
                    std::cout << "Asset Value " << id << " has been created" << std::endl;
                } else if(subsubcommand == "show"){
                    budget::show_asset_values(w);
                } else if(subsubcommand == "small"){
                    budget::small_show_asset_values(w);
                } else if(subsubcommand == "list"){
                    list_asset_values(w);
                } else if(subsubcommand == "edit"){
                    size_t id = 0;

                    enough_args(args, 4);

                    id = to_number<size_t>(args[3]);

                    if (!asset_values.exists(id)) {
                        throw budget_exception("There are no asset values with id " + args[3]);
                    }

                    auto& value = asset_values[id];

                    std::string asset_name = get_asset(value.asset_id).name;
                    edit_string_complete(asset_name, "Asset", get_asset_names(), not_empty_checker(), asset_checker());
                    value.asset_id = get_asset(asset_name).id;

                    edit_money(value.amount, "Amount", not_negative_checker());
                    edit_date(value.set_date, "Date");

                    if (asset_values.edit(value)) {
                        std::cout << "Asset Value " << id << " has been modified" << std::endl;
                    }
                } else if (subsubcommand == "delete") {
                    size_t id = 0;

                    enough_args(args, 4);

                    id = to_number<size_t>(args[3]);

                    if (!assets.exists(id)) {
                        throw budget_exception("There are no asset value with id " + args[2]);
                    }

                    asset_values.remove(id);

                    std::cout << "Asset value " << id << " has been deleted" << std::endl;
                } else {
                    throw budget_exception("Invalid subcommand value \"" + subsubcommand + "\"");
                }
            }
        } else if (subcommand == "share") {
            if (args.size() == 2) {
                list_asset_shares(w);
            } else {
                auto& subsubcommand = args[2];

                if (subsubcommand == "add") {
                    if (get_share_asset_names().empty()) {
                        throw budget_exception("There are no asset with shares, create one first");
                    }

                    asset_share asset_share;
                    asset_share.guid = generate_guid();

                    std::string asset_name;
                    edit_string_complete(asset_name, "Asset", get_share_asset_names(), not_empty_checker(), share_asset_checker());
                    asset_share.asset_id = get_asset(asset_name).id;

                    asset_share.shares = 0;
                    edit_number(asset_share.shares, "Shares");

                    asset_share.price = 0;
                    edit_money(asset_share.price, "Price", not_negative_checker());

                    asset_share.date = budget::local_day();
                    edit_date(asset_share.date, "Date");

                    auto id = asset_shares.add(std::move(asset_share));
                    std::cout << "Asset Share " << id << " has been created" << std::endl;
                } else if (subsubcommand == "list") {
                    list_asset_shares(w);
                } else if (subsubcommand == "test") {
                    enough_args(args, 4);

                    auto quote = args[3];

                    std::cout << quote << ":" << budget::share_price(quote) << std::endl;
                } else if(subsubcommand == "edit"){
                    enough_args(args, 4);

                    size_t id = to_number<size_t>(args[3]);

                    if (!asset_shares.exists(id)) {
                        throw budget_exception("There are no asset share with id " + args[3]);
                    }

                    auto& share = asset_shares[id];

                    std::string asset_name = get_asset(share.asset_id).name;
                    edit_string_complete(asset_name, "Asset", get_share_asset_names(), not_empty_checker(), share_asset_checker());
                    share.asset_id = get_asset(asset_name).id;

                    edit_number(share.shares, "Shares");
                    edit_money(share.price, "Price", not_negative_checker());
                    edit_date(share.date, "Date");

                    if (asset_shares.edit(share)) {
                        std::cout << "Asset Share " << id << " has been modified" << std::endl;
                    }
                } else if (subsubcommand == "delete") {
                    enough_args(args, 4);

                    size_t id = to_number<size_t>(args[3]);

                    if (!asset_shares.exists(id)) {
                        throw budget_exception("There are no asset share with id " + args[3]);
                    }

                    asset_shares.remove(id);

                    std::cout << "Asset share " << id << " has been deleted" << std::endl;
                } else {
                    throw budget_exception("Invalid subcommand share \"" + subsubcommand + "\"");
                }
            }
        } else if (subcommand == "distribution") {
            auto& desired = get_desired_allocation();

            do {
                for (auto & clas : all_asset_classes()) {
                    budget::money alloc = get_asset_class_allocation(desired, clas);
                    edit_money(alloc, clas.name);
                    update_asset_class_allocation(desired, clas, alloc);
                }

                if (desired.total_allocation() != money(100)) {
                    std::cout << "The distribution must account to 100%" << std::endl;
                }
            } while (desired.total_allocation() != money(100));

            if (assets.edit(desired)) {
                std::cout << "The distribution has been modified" << std::endl;
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_assets(){
    asset_classes.load();
    assets.load();
    asset_values.load();
    asset_shares.load();
}

void budget::save_assets(){
    asset_classes.save();
    assets.save();
    asset_values.save();
    asset_shares.save();
}

budget::asset_class& budget::get_asset_class(size_t id){
    return asset_classes[id];
}

budget::asset_class& budget::get_asset_class(const std::string & name){
    for (auto& c : asset_classes.data) {
        if (c.name == name) {
            return c;
        }
    }

    cpp_unreachable("The asset class does not exist");
}

budget::asset& budget::get_asset(size_t id){
    return assets[id];
}

budget::asset& budget::get_asset(std::string name){
    for(auto& asset : assets.data){
        if(asset.name == name){
            return asset;
        }
    }

    cpp_unreachable("The asset does not exist");
}

budget::asset& budget::get_desired_allocation(){
    for (auto& asset : assets.data) {
        if (asset.name == "DESIRED" && asset.currency == "DESIRED") {
            return asset;
        }
    }

    asset asset;
    asset.guid        = generate_guid();
    asset.name        = "DESIRED";
    asset.currency    = "DESIRED";
    asset.share_based = false;
    asset.portfolio   = false;

    auto id = assets.add(std::move(asset));
    return get_asset(id);
}

budget::asset_value& budget::get_asset_value(size_t id){
    return asset_values[id];
}

budget::asset_share& budget::get_asset_share(size_t id) {
    return asset_shares[id];
}

void budget::migrate_assets_4_to_5(){
    assets.load([](const std::vector<std::string>& parts, asset& asset){
        asset.id              = to_number<size_t>(parts[0]);
        asset.guid            = parts[1];
        asset.name            = parts[2];
        asset.int_stocks      = parse_money(parts[3]);
        asset.dom_stocks      = parse_money(parts[4]);
        asset.bonds           = parse_money(parts[5]);
        asset.cash            = parse_money(parts[6]);
        asset.currency        = parts[7];
        asset.portfolio       = to_number<size_t>(parts[8]);
        asset.portfolio_alloc = parse_money(parts[9]);

        if(asset.guid == "XXXXX"){
            asset.guid = generate_guid();
        }

        // Version 5 added support for shares
        asset.share_based = false;
    });

    set_assets_changed();

    assets.save();
}

void budget::migrate_assets_5_to_6(){
    asset_class class_int_stocks{0, generate_guid(), "Int. Stocks"};
    asset_class class_dom_stocks{0, generate_guid(), "Dom. Stocks"};
    asset_class class_bonds{0, generate_guid(), "Bonds"};
    asset_class class_cash{0, generate_guid(), "Cash"};

    asset_classes.add(class_int_stocks);
    asset_classes.add(class_dom_stocks);
    asset_classes.add(class_bonds);
    asset_classes.add(class_cash);

    // Load asset with version 5
    assets.load([&](const std::vector<std::string>& parts, asset& asset){
        asset.id              = to_number<size_t>(parts[0]);
        asset.guid            = parts[1];
        asset.name = parts[2];
        auto int_stocks      = parse_money(parts[3]);
        auto dom_stocks      = parse_money(parts[4]);
        auto bonds           = parse_money(parts[5]);
        auto cash            = parse_money(parts[6]);
        asset.currency        = parts[7];
        asset.portfolio       = to_number<size_t>(parts[8]);
        asset.portfolio_alloc = parse_money(parts[9]);
        asset.share_based     = to_number<size_t>(parts[10]);
        asset.ticker          = parts[11] == "EMPTY" ? "" : parts[11];

        // Version 6 added support for asset classes
        asset.classes.emplace_back(class_int_stocks.id, int_stocks);
        asset.classes.emplace_back(class_dom_stocks.id, dom_stocks);
        asset.classes.emplace_back(class_bonds.id, bonds);
        asset.classes.emplace_back(class_cash.id, cash);
    });

    set_asset_classes_changed();
    set_assets_changed();

    asset_classes.save();
    assets.save();
}

std::ostream& budget::operator<<(std::ostream& stream, const asset_class& asset){
    return stream << asset.id << ':' << asset.guid << ':' << asset.name << ':';
}

void budget::operator>>(const std::vector<std::string>& parts, asset_class& clas){
    clas.id   = to_number<size_t>(parts[0]);
    clas.guid = parts[1];
    clas.name = parts[2];

    if (clas.guid == "XXXXX") {
        clas.guid = generate_guid();
    }
}

std::ostream& budget::operator<<(std::ostream& stream, const asset& asset){
    std::string classes;

    for (auto& allocation : asset.classes) {
        classes += budget::to_string(allocation.first) + ";" + budget::to_string(allocation.second) + ";";
    }

    if (classes.empty()) {
        classes = "EMPTY";
    }

    return stream << asset.id << ':' << asset.guid << ':' << asset.name << ':'
                  << asset.currency << ":" << asset.portfolio << ":" << asset.portfolio_alloc << ":"
                  << asset.share_based << ":" << (asset.ticker.empty() ? "EMPTY" : asset.ticker) << ":" << classes;
}

void budget::operator>>(const std::vector<std::string>& parts, asset& asset){
    bool random = config_contains("random");

    asset.id              = to_number<size_t>(parts[0]);
    asset.guid            = parts[1];
    asset.currency        = parts[3];
    asset.portfolio       = to_number<size_t>(parts[4]);
    asset.portfolio_alloc = parse_money(parts[5]);
    asset.share_based     = to_number<size_t>(parts[6]);
    asset.ticker          = parts[7] == "EMPTY" ? "" : parts[7];

    if(asset.guid == "XXXXX"){
        asset.guid = generate_guid();
    }

    if (random) {
        asset.name = parts[2];

        if (!(asset.name == "DESIRED" && asset.currency == "DESIRED")) {
            asset.name = budget::random_name(5);
        }
    } else {
        asset.name = parts[2];
    }

    auto assets_parts = split(parts[8], ';');
    for (size_t i = 0; i + 1 < assets_parts.size(); i += 2) {
        auto id    = to_number<size_t>(assets_parts[i]);
        auto alloc = parse_money(assets_parts[i + 1]);
        asset.classes.emplace_back(id, alloc);
    }
}

std::ostream& budget::operator<<(std::ostream& stream, const asset_value& asset_value){
    return stream
               << asset_value.id
        << ':' << asset_value.guid
        << ':' << asset_value.asset_id
        << ":" << asset_value.amount
        << ":" << to_string(asset_value.set_date);
}

void budget::operator>>(const std::vector<std::string>& parts, asset_value& asset_value){
    bool random = config_contains("random");

    asset_value.id       = to_number<size_t>(parts[0]);
    asset_value.guid     = parts[1];
    asset_value.asset_id = to_number<size_t>(parts[2]);
    asset_value.set_date = from_string(parts[4]);

    if(asset_value.guid == "XXXXX"){
        asset_value.guid = generate_guid();
    }

    if (random) {
        asset_value.amount = budget::random_money(1000, 50000);
    } else {
        asset_value.amount = parse_money(parts[3]);
    }
}

std::ostream& budget::operator<<(std::ostream& stream, const asset_share& asset_share){
    return stream
               << asset_share.id
        << ':' << asset_share.guid
        << ':' << asset_share.asset_id
        << ":" << asset_share.shares
        << ":" << to_string(asset_share.date)
        << ":" << asset_share.price;
}

void budget::operator>>(const std::vector<std::string>& parts, asset_share& asset_share){
    bool random = config_contains("random");

    asset_share.id       = to_number<size_t>(parts[0]);
    asset_share.guid     = parts[1];
    asset_share.asset_id = to_number<size_t>(parts[2]);
    asset_share.date     = from_string(parts[4]);
    asset_share.price    = parse_money(parts[5]);

    if (asset_share.guid == "XXXXX") {
        asset_share.guid = generate_guid();
    }

    if (random) {
        static std::random_device rd;
        static std::mt19937_64 engine(rd());

        std::uniform_int_distribution<int> dist(0, 1000);

        asset_share.shares = dist(engine);
    } else {
        asset_share.shares = to_number<size_t>(parts[3]);
    }
}

bool budget::asset_class_exists(const std::string& name){
    for (auto& clas : asset_classes.data) {
        if (clas.name == name) {
            return true;
        }
    }

    return false;
}

bool budget::asset_exists(const std::string& name){
    for (auto& asset : assets.data) {
        if (asset.name == name) {
            return true;
        }
    }

    return false;
}

bool budget::share_asset_exists(const std::string& name){
    for (auto& asset : assets.data) {
        if (asset.name == name) {
            return asset.share_based;
        }
    }

    return false;
}

std::vector<asset_class>& budget::all_asset_classes(){
    return asset_classes.data;
}

std::vector<asset>& budget::all_assets(){
    return assets.data;
}

std::vector<asset_value>& budget::all_asset_values(){
    return asset_values.data;
}

budget::date budget::asset_start_date(const budget::asset& asset) {
    budget::date start = budget::local_day();

    if (asset.share_based) {
        for (auto & share : all_asset_shares()) {
            if (share.asset_id == asset.id) {
                start = std::min(share.date, start);
            }
        }
   } else {
       for (auto & value : all_asset_values()) {
           if (value.asset_id == asset.id) {
               start = std::min(value.set_date, start);
           }
       }
   }

    return start;
}

budget::date budget::asset_start_date() {
    budget::date start = budget::local_day();

    //TODO If necessary, avoid double loops

    for (auto & asset : all_user_assets()) {
        start = std::min(asset_start_date(asset), start);
    }

    return start;
}

std::vector<asset_share>& budget::all_asset_shares(){
    return asset_shares.data;
}

void budget::set_asset_classes_changed(){
    asset_classes.set_changed();
}

void budget::set_assets_changed(){
    assets.set_changed();
}

void budget::set_asset_values_changed(){
    asset_values.set_changed();
}

void budget::set_asset_shares_changed(){
    asset_shares.set_changed();
}

void budget::set_asset_class_next_id(size_t next_id){
    asset_classes.next_id = next_id;
}

void budget::set_assets_next_id(size_t next_id){
    assets.next_id = next_id;
}

void budget::set_asset_values_next_id(size_t next_id){
    asset_values.next_id = next_id;
}

void budget::set_asset_shares_next_id(size_t next_id){
    asset_shares.next_id = next_id;
}

std::string budget::get_default_currency(){
    if(budget::config_contains("default_currency")){
        return budget::config_value("default_currency");
    }

    return "CHF";
}

std::string to_percent(double p){
    std::stringstream ss;

    ss << std::setprecision(4) << p << "%";

    return ss.str();
}

void budget::show_assets(budget::writer& w){
    if (!assets.data.size()) {
        w << "No assets" << end_of_line;
        return;
    }

    w << title_begin << "Assets " << add_button("assets") << title_end;

    std::vector<std::string> columns = {"ID", "Name"};

    for (auto & clas : all_asset_classes()) {
        columns.emplace_back(clas.name);
    }

    columns.emplace_back("Currency");
    columns.emplace_back("Portfolio");
    columns.emplace_back("Alloc");
    columns.emplace_back("Share");
    columns.emplace_back("Ticker");
    columns.emplace_back("Edit");

    std::vector<std::vector<std::string>> contents;

    // Display the assets

    for(auto& asset : assets.data){
        if(asset.name == "DESIRED" && asset.currency == "DESIRED"){
            continue;
        }

        std::vector<std::string> line;

        line.emplace_back(to_string(asset.id));
        line.emplace_back(asset.name);

        for (auto& clas : all_asset_classes()) {
            bool found = false;

            for (auto& c : asset.classes) {
                if (c.first == clas.id) {
                    line.emplace_back(to_string(c.second));
                    found = true;
                    break;
                }
            }

            if (!found) {
                line.emplace_back("0.00");
            }
        }

        line.emplace_back(to_string(asset.currency));
        line.emplace_back(asset.portfolio ? "Yes" : "No");
        line.emplace_back(asset.portfolio ? to_string(asset.portfolio_alloc) : "");
        line.emplace_back(asset.share_based ? "Yes" : "No");
        line.emplace_back(asset.share_based ? asset.ticker : "");
        line.emplace_back("::edit::assets::" + budget::to_string(asset.id));

        contents.emplace_back(std::move(line));
    }

    w.display_table(columns, contents);
}

void budget::show_asset_portfolio(budget::writer& w){
    if (!asset_values.data.size() && !asset_shares.data.size()) {
        w << "No asset values nor shares" << end_of_line;
        return;
    }

    w << title_begin << "Portfolio" << title_end;

    std::vector<std::string> columns = {"Name", "Total", "Currency", "Converted", "Allocation"};
    std::vector<std::vector<std::string>> contents;

    budget::money total;

    for (auto& asset : all_user_assets()) {
        if (asset.portfolio) {
            total += get_asset_value_conv(asset);
        }
    }

    for(auto& asset : all_user_assets()){
        if (asset.portfolio) {
            auto amount = get_asset_value(asset);

            if (amount) {
                auto conv_amount  = amount * exchange_rate(asset.currency);
                auto allocation   = 100.0 * (conv_amount / total);

                contents.push_back({asset.name,
                                    to_string(amount),
                                    asset.currency,
                                    to_string(conv_amount),
                                    to_percent(allocation)});
            }
        }
    }

    contents.push_back({ "", "", "", "", ""});
    contents.push_back({ "Total", budget::to_string(total), get_default_currency(), "", ""});

    w.display_table(columns, contents, 1, {}, 1, 2);
}

void budget::show_asset_rebalance(budget::writer& w, bool nocash){
    if (!asset_values.data.size() && !asset_shares.data.size()) {
        w << "No asset values" << end_of_line;
        return;
    }

    w << title_begin << "Rebalancing" << title_end;

    std::vector<std::string> columns = {"Name", "Total", "Currency", "Converted", "Allocation", "Desired Allocation", "Desired Total", "Difference"};
    std::vector<std::vector<std::string>> contents;

    budget::money total;

    for (auto& asset : all_user_assets()) {
        if (asset.portfolio) {
            if (nocash && asset.is_cash()) {
                continue;
            }

            total += get_asset_value_conv(asset);
        }
    }

    budget::money total_rebalance;

    for (auto& asset : all_user_assets()) {
        if (asset.portfolio) {
            if (nocash && asset.is_cash()) {
                continue;
            }

            auto amount = get_asset_value(asset);

            if (amount.zero() && asset.portfolio_alloc.zero()) {
                continue;
            }

            auto conv_amount = amount * exchange_rate(asset.currency);
            auto allocation  = 100.0 * (conv_amount / total);
            auto desired     = total * (float(asset.portfolio_alloc) / 100.0);
            auto difference  = desired - conv_amount;

            total_rebalance += difference.abs();

            if (amount || difference) {
                contents.push_back({
                    asset.name,
                    to_string(amount),
                    asset.currency,
                    to_string(conv_amount),
                    to_percent(allocation),
                    to_string(asset.portfolio_alloc),
                    to_string(desired),
                    format_money(difference)
                });
            }
        }
    }

    // Display the total rebalancing effort
    contents.push_back({"", "", "", "", "", "", "", ""});
    contents.push_back({"Total effort", "", "", "", "", "", "", format_money(total_rebalance)});

    w.display_table(columns, contents, 1, {}, 1, 2);
}

void budget::small_show_asset_values(budget::writer& w){
    if (!asset_values.data.size() && !asset_shares.data.size()) {
        w << "No asset values" << end_of_line;
        return;
    }

    w << title_begin << "Net Worth" << title_end;

    std::vector<std::string> columns = {"Name", "Value", "Currency"};
    std::vector<std::vector<std::string>> contents;

    budget::money total;

    for (auto& asset : all_user_assets()) {
        auto amount = get_asset_value(asset);

        if (amount) {
            contents.push_back({asset.name,
                                to_string(amount),
                                asset.currency});

            total += amount * exchange_rate(asset.currency);
        }
    }

    contents.push_back({"", "", ""});
    contents.push_back({"Net Worth", budget::to_string(total), get_default_currency()});

    // Display the table

    w.display_table(columns, contents, 1, {}, 1);
}

void budget::show_asset_values(budget::writer& w){
    if (!asset_values.data.size() && !asset_shares.data.size()) {
        w << "No asset values" << end_of_line;
        return;
    }

    w << title_begin << "Net Worth" << title_end;

    std::vector<std::string> columns = {"Name"};

    for (auto & clas : all_asset_classes()) {
        columns.emplace_back(clas.name);
    }

    columns.emplace_back("Total");
    columns.emplace_back("Currency");

    std::vector<std::vector<std::string>> contents;

    std::map<std::string, budget::money> classes;

    budget::money total;

    for(auto& asset : all_user_assets()){
        auto amount = get_asset_value(asset);

        if (amount) {
            std::vector<std::string> line;

            line.emplace_back(asset.name);

            for (auto& clas : all_asset_classes()) {
                bool found = false;

                for (auto& c : asset.classes) {
                    if (c.first == clas.id) {
                        auto class_amount = amount  * (float(c.second) / 100.0);
                        line.emplace_back(to_string(class_amount));
                        classes[clas.name] += class_amount * exchange_rate(asset.currency);
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    line.emplace_back("0.00");
                }
            }

            line.emplace_back(to_string(amount));
            line.emplace_back(asset.currency);

            contents.emplace_back(std::move(line));

            total += amount * exchange_rate(asset.currency);
        }
    }

    contents.emplace_back(columns.size(), "");

    {
        std::vector<std::string> line;

        line.emplace_back("Total");

        for (auto& clas : all_asset_classes()) {
            if (classes.count(clas.name)) {
                line.emplace_back(budget::to_string(classes[clas.name]));
            } else {
                line.emplace_back("0.00");
            }
        }

        line.emplace_back(budget::to_string(total));
        line.emplace_back(budget::get_default_currency());

        contents.emplace_back(std::move(line));
    }

    contents.emplace_back(columns.size(), "");

    {
        std::vector<std::string> line;

        line.emplace_back("Distribution");

        for (auto& clas : all_asset_classes()) {
            if (classes.count(clas.name)) {
                auto amount = classes[clas.name];
                line.emplace_back(budget::to_string_precision(100 * amount.dollars() / (double) total.dollars(), 2));
            } else {
                line.emplace_back("0");
            }
        }

        line.emplace_back("100");
        line.emplace_back("");

        contents.emplace_back(std::move(line));
    }

    decltype(auto) desired = get_desired_allocation();

    if (desired.total_allocation()) {
        std::vector<std::string> line1;
        std::vector<std::string> line2;
        std::vector<std::string> line3;

        line1.emplace_back("Desired Distribution");
        line2.emplace_back("Desired Total");
        line3.emplace_back("Difference (need)");

        for (auto& clas : all_asset_classes()) {
            budget::money desired_alloc;

            for (auto& c : desired.classes) {
                if (c.first == clas.id) {
                    desired_alloc = c.second;
                    break;
                }
            }

            line1.emplace_back(to_string(desired_alloc));
            line2.emplace_back(to_string(total * (float(desired_alloc) / 100.0)));

            if (classes.count(clas.name)) {
                line3.emplace_back(to_string(total * (float(desired_alloc) / 100.0) - classes[clas.name]));
            } else {
                line3.emplace_back(to_string(total * (float(desired_alloc) / 100.0)));
            }
        }

        line1.emplace_back("100");
        line1.emplace_back("");

        line2.emplace_back(to_string(total));
        line2.emplace_back(get_default_currency());

        line3.emplace_back("0.00");
        line3.emplace_back(get_default_currency());

        contents.emplace_back(std::move(line1));
        contents.emplace_back(std::move(line2));
        contents.emplace_back(std::move(line3));
    }

    {
        contents.emplace_back(columns.size(), "");
    }

    {
        std::vector<std::string> line(columns.size(), "");
        line[0] = "Net Worth";
        line[line.size() - 2] = budget::to_string(total);
        line[line.size() - 1] = get_default_currency();
        contents.emplace_back(std::move(line));

    }

    // Display the table

    if (desired.total_allocation()) {
        w.display_table(columns, contents, 1, {contents.size() - 8, contents.size() - 3}, 1);
    } else {
        w.display_table(columns, contents, 1, {}, 1);
    }
}

bool budget::asset_class_exists(size_t id){
    return asset_classes.exists(id);
}

void budget::asset_class_delete(size_t id) {
    if (!asset_classes.exists(id)) {
        throw budget_exception("There are no class asset with id ");
    }

    asset_classes.remove(id);
}

asset_class& budget::asset_class_get(size_t id) {
    if (!asset_classes.exists(id)) {
        throw budget_exception("There are no asset class with id ");
    }

    return asset_classes[id];
}

void budget::add_asset_class(budget::asset_class&& asset){
    asset_classes.add(std::forward<budget::asset_class>(asset));
}

bool budget::asset_exists(size_t id){
    return assets.exists(id);
}

void budget::asset_delete(size_t id) {
    if (!assets.exists(id)) {
        throw budget_exception("There are no asset with id ");
    }

    assets.remove(id);
}

asset& budget::asset_get(size_t id) {
    if (!assets.exists(id)) {
        throw budget_exception("There are no asset with id ");
    }

    return assets[id];
}

void budget::add_asset(budget::asset&& asset){
    assets.add(std::forward<budget::asset>(asset));
}

bool budget::asset_value_exists(size_t id){
    return asset_values.exists(id);
}

void budget::asset_value_delete(size_t id) {
    if (!asset_values.exists(id)) {
        throw budget_exception("There are no asset_value with id ");
    }

    asset_values.remove(id);
}

asset_value& budget::asset_value_get(size_t id) {
    if (!asset_values.exists(id)) {
        throw budget_exception("There are no asset_value with id ");
    }

    return asset_values[id];
}

void budget::add_asset_value(budget::asset_value&& asset_value){
    asset_values.add(std::forward<budget::asset_value>(asset_value));
}

bool budget::asset_share_exists(size_t id){
    return asset_shares.exists(id);
}

void budget::asset_share_delete(size_t id) {
    if (!asset_shares.exists(id)) {
        throw budget_exception("There are no asset_share with id ");
    }

    asset_shares.remove(id);
}

asset_share& budget::asset_share_get(size_t id) {
    if (!asset_shares.exists(id)) {
        throw budget_exception("There are no asset_share with id ");
    }

    return asset_shares[id];
}

void budget::add_asset_share(budget::asset_share&& asset_share){
    asset_shares.add(std::forward<budget::asset_share>(asset_share));
}

void budget::list_asset_values(budget::writer& w){
    if (!asset_values.data.size()) {
        w << "No asset values" << end_of_line;
        return;
    }

    std::vector<std::string> columns = {"ID", "Asset", "Amount", "Date", "Edit"};
    std::vector<std::vector<std::string>> contents;

    // Display the asset values

    for(auto& value : asset_values.data){
        contents.push_back({to_string(value.id), get_asset(value.asset_id).name, to_string(value.amount), to_string(value.set_date), "::edit::asset_values::" + budget::to_string(value.id)});
    }

    w.display_table(columns, contents);
}

void budget::list_asset_shares(budget::writer& w){
    if (!asset_shares.data.size()) {
        w << "No asset shares" << end_of_line;
        return;
    }

    std::vector<std::string> columns = {"ID", "Asset", "Shares", "Date", "Price", "Edit"};
    std::vector<std::vector<std::string>> contents;

    // Display the asset values

    for (auto& value : asset_shares.data) {
        contents.push_back({to_string(value.id), get_asset(value.asset_id).name,
                            to_string(value.shares), to_string(value.date), to_string(value.price),
                            "::edit::asset_shares::" + budget::to_string(value.id)});
    }

    w.display_table(columns, contents);
}

budget::money budget::get_portfolio_value(){
    budget::money total;

    for (auto & asset : all_user_assets()) {
        if (asset.portfolio) {
            total += get_asset_value_conv(asset);
        }
    }

    return total;
}

budget::money budget::get_net_worth(){
    return get_net_worth(budget::local_day());
}

budget::money budget::get_net_worth(budget::date d){
    budget::money total;

    for (auto & asset : all_user_assets()) {
        total += get_asset_value_conv(asset, d);
    }

    return total;
}

budget::money budget::get_net_worth_cash(){
    budget::money total;

    for (auto & asset : all_user_assets()) {
        if (asset.cash == budget::money(100)) {
            total += get_asset_value_conv(asset);
        }
    }

    return total;
}

budget::money budget::get_asset_value(budget::asset & asset, budget::date d) {
    if (asset.share_based) {
        int64_t shares = 0;

        for (auto& asset_share : asset_shares.data) {
            if (asset_share.asset_id == asset.id) {
                if (asset_share.date <= d) {
                    shares += asset_share.shares;
                }
            }
        }

        if (shares > 1) {
            return budget::money(shares) * share_price(asset.ticker, d);
        }
    } else {
        size_t asset_value_id  = 0;
        bool asset_value_found = false;

        for (auto& asset_value : asset_values.data) {
            if (asset_value.asset_id == asset.id) {
                if (asset_value.set_date <= d) {
                    if (!asset_value_found) {
                        asset_value_id = asset_value.id;
                    } else if (asset_value.set_date >= get_asset_value(asset_value_id).set_date) {
                        asset_value_id = asset_value.id;
                    }

                    asset_value_found = true;
                }
            }
        }

        if (asset_value_found) {
            return get_asset_value(asset_value_id).amount;
        }
    }

    return {};
}

budget::money budget::get_asset_value(budget::asset & asset) {
    return get_asset_value(asset, budget::local_day());
}

budget::money budget::get_asset_value_conv(budget::asset & asset) {
    return get_asset_value_conv(asset, budget::local_day());
}

budget::money budget::get_asset_value_conv(budget::asset & asset, budget::date d) {
    auto amount = get_asset_value(asset, d);
    return amount * exchange_rate(asset.currency, d);
}

budget::money budget::get_asset_value_conv(budget::asset & asset, const std::string& currency) {
    return get_asset_value_conv(asset, budget::local_day(), currency);
}

budget::money budget::get_asset_value_conv(budget::asset & asset, budget::date d, const std::string& currency) {
    auto amount = get_asset_value(asset, d);
    return amount * exchange_rate(asset.currency, currency, d);
}

budget::money budget::get_asset_class_allocation(budget::asset& asset, budget::asset_class & clas) {
    for (auto & c : asset.classes) {
        if (c.first == clas.id) {
            return c.second;
        }
    }

    return {};
}

