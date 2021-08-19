//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
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

#include "data_cache.hpp"
#include "assets.hpp"
#include "liabilities.hpp"
#include "budget_exception.hpp"
#include "args.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "console.hpp"
#include "writer.hpp"
#include "currency.hpp"
#include "share.hpp"

using namespace budget;

namespace {

static data_handler<asset> assets { "assets", "assets.data" };

std::vector<std::string> get_asset_names(data_cache& cache) {
    std::vector<std::string> asset_names;

    for (auto& asset : cache.user_assets()) {
        asset_names.push_back(asset.name);
    }

    return asset_names;
}

std::vector<std::string> get_share_asset_names(data_cache& cache) {
    std::vector<std::string> asset_names;

    for (auto& asset : cache.user_assets()) {
        if (asset.share_based) {
            asset_names.push_back(asset.name);
        }
    }

    return asset_names;
}

} //end of anonymous namespace

std::map<std::string, std::string> budget::asset::get_params() const {
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
    for (auto & [clas_id, alloc] : classes) {
        params["input_class_" + to_string(clas_id)] = budget::to_string(alloc);
    }

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

            auto asset_classes = all_asset_classes();

            do {
                for (auto & clas : asset_classes) {
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

            auto id = assets.add(asset);
            std::cout << "Asset " << id << " has been created" << std::endl;
        } else if(subcommand == "delete"){
            size_t id = 0;

            if(args.size() >= 3){
                enough_args(args, 3);

                id = to_number<size_t>(args[2]);
            } else {
                std::string name;
                edit_string_complete(name, "Asset", get_asset_names(w.cache), not_empty_checker(), asset_checker());

                id = budget::get_asset(name).id;
            }

            auto asset = get_asset(id);

            if (asset.name == "DESIRED" && asset.currency == "DESIRED") {
                throw budget_exception("Cannot delete special asset " + args[2]);
            }

            for (auto& value : all_asset_values()) {
                if (!value.liability && value.asset_id == id) {
                    throw budget_exception("There are still asset values linked to asset " + args[2]);
                }
            }

            for (auto& share : all_asset_shares()) {
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
            } else {
                std::string name;
                edit_string_complete(name, "Asset", get_asset_names(w.cache), not_empty_checker(), asset_checker());

                id = get_asset(name).id;
            }

            auto asset = assets[id];

            edit_string(asset.name, "Name", not_empty_checker());

            //Verify that there are no OTHER asset with this name
            //in the current set of assets (taking archiving into asset)

            for (auto& other_asset : w.cache.user_assets()) {
                if (other_asset.id != id) {
                    if (other_asset.name == asset.name) {
                        throw budget_exception("There is already an asset with the name " + asset.name);
                    }
                }
            }

            do {
                for (auto & clas : w.cache.asset_classes()) {
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

            if (assets.indirect_edit(asset)) {
                std::cout << "Asset " << id << " has been modified" << std::endl;
            }
        } else if(subcommand == "class"){
            if(args.size() == 2){
                budget::show_asset_classes(w);
            } else {
                auto& subsubcommand = args[2];

                if(subsubcommand == "add"){
                    asset_class clas;
                    clas.guid = generate_guid();

                    edit_string(clas.name, "Name", not_empty_checker());

                    if (asset_class_exists(clas.name)) {
                        throw budget_exception("This asset class already exists");
                    }

                    auto id = add_asset_class(clas);
                    std::cout << "Asset Clas " << id << " has been created" << std::endl;
                } else if(subsubcommand == "edit"){
                    size_t id = 0;

                    enough_args(args, 4);

                    id = to_number<size_t>(args[3]);

                    if (!asset_class_exists(id)) {
                        throw budget_exception("This asset class does not exist");
                    }

                    auto clas = get_asset_class(id);

                    edit_string(clas.name, "Name", not_empty_checker());

                    for (auto & other_class : all_asset_classes()) {
                        if (other_class.id != id && other_class.name == clas.name) {
                            throw budget_exception("This asset class already exists");
                        }
                    }

                    if (edit_asset_class(clas)) {
                        std::cout << "Asset Class " << id << " has been modified" << std::endl;
                    }
                } else if (subsubcommand == "delete") {
                    size_t id = 0;

                    enough_args(args, 4);

                    id = to_number<size_t>(args[3]);

                    if (!asset_class_exists(id)) {
                        throw budget_exception("This asset class does not exist");
                    }

                    auto clas = get_asset_class(id);

                    for (auto & asset : all_assets()) {
                        if (get_asset_class_allocation(asset, clas)) {
                            throw budget_exception("Cannot delete an asset class that is still used");
                        }
                    }

                    asset_class_delete(id);

                    std::cout << "Asset class " << id << " has been deleted" << std::endl;
                } else if(subsubcommand == "show"){
                    budget::show_asset_classes(w);
                } else {
                    throw budget_exception("Invalid subcommand value \"" + subsubcommand + "\"");
                }
            }
        } else if(subcommand == "value"){
            if(args.size() == 2){
                budget::show_asset_values(w);
            } else {
                auto& subsubcommand = args[2];

                if(subsubcommand == "set"){
                    asset_value asset_value;
                    asset_value.guid = generate_guid();
                    asset_value.liability = false;

                    std::string asset_name;
                    edit_string_complete(asset_name, "Asset", get_asset_names(w.cache), not_empty_checker(), asset_checker());
                    asset_value.asset_id = get_asset(asset_name).id;

                    edit_money(asset_value.amount, "Amount", not_negative_checker());

                    asset_value.set_date = budget::local_day();
                    edit_date(asset_value.set_date, "Date");

                    auto id = add_asset_value(asset_value);
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

                    if (!asset_value_exists(id)) {
                        throw budget_exception("There are no asset values with id " + args[3]);
                    }

                    auto value = get_asset_value(id);

                    if (value.liability) {
                        throw budget_exception("Cannot edit liability value from the asset module");
                    }

                    std::string asset_name = get_asset(value.asset_id).name;
                    edit_string_complete(asset_name, "Asset", get_asset_names(w.cache), not_empty_checker(), asset_checker());
                    value.asset_id = get_asset(asset_name).id;

                    edit_money(value.amount, "Amount", not_negative_checker());
                    edit_date(value.set_date, "Date");

                    if (edit_asset_value(value)) {
                        std::cout << "Asset Value " << id << " has been modified" << std::endl;
                    }
                } else if (subsubcommand == "delete") {
                    enough_args(args, 4);

                    size_t id = to_number<size_t>(args[3]);

                    auto value = get_asset_value(id);

                    if (value.liability) {
                        throw budget_exception("Cannot edit liability value from the asset module");
                    }

                    asset_value_delete(id);

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
                    if (get_share_asset_names(w.cache).empty()) {
                        throw budget_exception("There are no asset with shares, create one first");
                    }

                    asset_share asset_share;
                    asset_share.guid = generate_guid();

                    std::string asset_name;
                    edit_string_complete(asset_name, "Asset", get_share_asset_names(w.cache), not_empty_checker(), share_asset_checker());
                    asset_share.asset_id = get_asset(asset_name).id;

                    asset_share.shares = 0;
                    edit_number(asset_share.shares, "Shares");

                    asset_share.price = 0;
                    edit_money(asset_share.price, "Price", not_negative_checker());

                    asset_share.date = budget::local_day();
                    edit_date(asset_share.date, "Date");

                    auto id = add_asset_share(asset_share);
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

                    if (!asset_share_exists(id)) {
                        throw budget_exception("There are no asset share with id " + args[3]);
                    }

                    auto share = get_asset_share(id);

                    std::string asset_name = get_asset(share.asset_id).name;
                    edit_string_complete(asset_name, "Asset", get_share_asset_names(w.cache), not_empty_checker(), share_asset_checker());
                    share.asset_id = get_asset(asset_name).id;

                    edit_number(share.shares, "Shares");
                    edit_money(share.price, "Price", not_negative_checker());
                    edit_date(share.date, "Date");

                    if (edit_asset_share(share)) {
                        std::cout << "Asset Share " << id << " has been modified" << std::endl;
                    }
                } else if (subsubcommand == "delete") {
                    enough_args(args, 4);

                    size_t id = to_number<size_t>(args[3]);

                    if (!asset_share_exists(id)) {
                        throw budget_exception("There are no asset share with id " + args[3]);
                    }

                    asset_share_delete(id);

                    std::cout << "Asset share " << id << " has been deleted" << std::endl;
                } else {
                    throw budget_exception("Invalid subcommand share \"" + subsubcommand + "\"");
                }
            }
        } else if (subcommand == "distribution") {
            auto desired = get_desired_allocation();

            auto asset_classes = all_asset_classes();

            do {
                for (auto & clas : asset_classes) {
                    budget::money alloc = get_asset_class_allocation(desired, clas);
                    edit_money(alloc, clas.name);
                    update_asset_class_allocation(desired, clas, alloc);
                }

                if (desired.total_allocation() != money(100)) {
                    std::cout << "The distribution must account to 100%" << std::endl;
                }
            } while (desired.total_allocation() != money(100));

            if (assets.indirect_edit(desired)) {
                std::cout << "The distribution has been modified" << std::endl;
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_assets(){
    load_asset_classes();
    assets.load();
    load_asset_values();
    load_asset_shares();
}

void budget::save_assets(){
    save_asset_classes();
    assets.save();
    save_asset_values();
    save_asset_shares();
}

budget::asset budget::get_asset(size_t id){
    return assets[id];
}

budget::asset budget::get_asset(std::string name){
    for(auto& asset : assets.data()){
        if(asset.name == name){
            return asset;
        }
    }

    cpp_unreachable("The asset does not exist");
}

budget::asset budget::get_desired_allocation(){
    for (auto& asset : assets.data()) {
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

    auto id = assets.add(asset);
    return get_asset(id);
}

void budget::migrate_assets_4_to_5(){
    assets.load([](data_reader & reader, asset& asset){
        reader >> asset.id;
        reader >> asset.guid;
        reader >> asset.name;
        reader >> asset.int_stocks;
        reader >> asset.dom_stocks;
        reader >> asset.bonds;
        reader >> asset.cash;
        reader >> asset.currency;
        reader >> asset.portfolio;
        reader >> asset.portfolio_alloc;

        if (asset.guid == "XXXXX") {
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

    add_asset_class(class_int_stocks);
    add_asset_class(class_dom_stocks);
    add_asset_class(class_bonds);
    add_asset_class(class_cash);

    // Load asset with version 5
    assets.load([&](data_reader & reader, asset& asset){
        reader >> asset.id;
        reader >> asset.guid;
        reader >> asset.name;
        reader >> asset.int_stocks;
        reader >> asset.dom_stocks;
        reader >> asset.bonds;
        reader >> asset.cash;
        reader >> asset.currency;
        reader >> asset.portfolio;
        reader >> asset.portfolio_alloc;
        reader >> asset.share_based;
        reader >> asset.ticker;

        if (asset.ticker == "EMPTY") {
            asset.ticker = "";
        }

        // Version 6 added support for asset classes
        asset.classes.emplace_back(class_int_stocks.id, asset.int_stocks);
        asset.classes.emplace_back(class_dom_stocks.id, asset.dom_stocks);
        asset.classes.emplace_back(class_bonds.id, asset.bonds);
        asset.classes.emplace_back(class_cash.id, asset.cash);
    });

    set_asset_classes_changed();
    set_assets_changed();

    save_asset_classes();
    assets.save();
}

void budget::asset::save(data_writer & writer){
    std::string classes_str;

    for (auto& [clas_id, alloc] : classes) {
        classes_str += budget::to_string(clas_id) + ";" + budget::to_string(alloc) + ";";
    }

    if (classes_str.empty()) {
        classes_str = "EMPTY";
    }

    writer << id;
    writer << guid;
    writer << name;
    writer << currency;
    writer << portfolio;
    writer << portfolio_alloc;
    writer << share_based;
    writer << std::string(ticker.empty() ? "EMPTY" : ticker);
    writer << classes_str;
}

void budget::asset::load(data_reader & reader){
    reader >> id;
    reader >> guid;
    reader >> name;
    reader >> currency;
    reader >> portfolio;
    reader >> portfolio_alloc;
    reader >> share_based;
    reader >> ticker;

    std::string assets_parts_str;
    reader >> assets_parts_str;

    auto assets_parts = split(assets_parts_str, ';');
    for (size_t i = 0; i + 1 < assets_parts.size(); i += 2) {
        auto id    = to_number<size_t>(assets_parts[i]);
        auto alloc = money_from_string(assets_parts[i + 1]);
        classes.emplace_back(id, alloc);
    }

    if (ticker == "EMPTY") {
        ticker = "";
    }

    if (config_contains("random")) {
        if (!(name == "DESIRED" && currency == "DESIRED")) {
            name = budget::random_name(5);
        }
    }
}

bool budget::asset_exists(const std::string& name){
    for (auto& asset : assets.data()) {
        if (asset.name == name) {
            return true;
        }
    }

    return false;
}

bool budget::share_asset_exists(const std::string& name){
    for (auto& asset : assets.data()) {
        if (asset.name == name) {
            return asset.share_based;
        }
    }

    return false;
}

std::vector<asset> budget::all_assets(){
    return assets.data();
}

budget::date budget::asset_start_date(data_cache & cache, const budget::asset& asset) {
    budget::date start = budget::local_day();

    if (asset.share_based) {
        for (auto & share : cache.asset_shares()) {
            if (share.asset_id == asset.id) {
                start = std::min(share.date, start);
            }
        }
   } else {
       for (auto & value : cache.asset_values()) {
           if (!value.liability && value.asset_id == asset.id) {
               start = std::min(value.set_date, start);
           }
       }
   }

    return start;
}

budget::date budget::asset_start_date(data_cache & cache) {
    budget::date start = budget::local_day();

    //TODO If necessary, avoid double loops

    for (auto & asset : cache.user_assets()) {
        start = std::min(asset_start_date(cache, asset), start);
    }

    return start;
}

void budget::set_assets_changed(){
    assets.set_changed();
}

void budget::set_assets_next_id(size_t next_id){
    assets.next_id = next_id;
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
    if (!assets.size()) {
        w << "No assets" << end_of_line;
        return;
    }

    w << title_begin << "Assets " << add_button("assets") << title_end;

    std::vector<std::string> columns = {"ID", "Name"};

    auto asset_classes = all_asset_classes();

    for (auto & clas : asset_classes) {
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

    for(auto& asset : assets.data()){
        if(asset.name == "DESIRED" && asset.currency == "DESIRED"){
            continue;
        }

        std::vector<std::string> line;

        line.emplace_back(to_string(asset.id));
        line.emplace_back(asset.name);

        for (auto& clas : asset_classes) {
            bool found = false;

            for (auto& [class_id, alloc] : asset.classes) {
                if (class_id == clas.id) {
                    line.emplace_back(to_string(alloc));
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
    if (no_asset_values() && no_asset_shares()) {
        w << "No asset values nor shares" << end_of_line;
        return;
    }

    w << title_begin << "Portfolio" << title_end;

    std::vector<std::string> columns = {"Name", "Total", "Currency", "Converted", "Allocation"};
    std::vector<std::vector<std::string>> contents;

    budget::money total;

    for (auto& asset : w.cache.user_assets()) {
        if (asset.portfolio) {
            total += get_asset_value_conv(asset, w.cache);
        }
    }

    for(auto& asset : w.cache.user_assets()){
        if (asset.portfolio) {
            auto amount = get_asset_value(asset, w.cache);

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
    if (no_asset_values() && no_asset_shares()) {
        w << "No asset values" << end_of_line;
        return;
    }

    w << title_begin << "Rebalancing" << title_end;

    std::vector<std::string> columns = {"Name", "Total", "Currency", "Converted", "Allocation", "Desired Allocation", "Desired Total", "Difference"};
    std::vector<std::vector<std::string>> contents;

    budget::money total;

    for (auto& asset : w.cache.user_assets()) {
        if (asset.portfolio) {
            if (nocash && asset.is_cash()) {
                continue;
            }

            total += get_asset_value_conv(asset, w.cache);
        }
    }

    budget::money total_rebalance;

    for (auto& asset : w.cache.user_assets()) {
        if (asset.portfolio) {
            if (nocash && asset.is_cash()) {
                continue;
            }

            auto amount = get_asset_value(asset, w.cache);

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
    if (no_asset_values() && no_asset_shares()) {
        w << "No asset values" << end_of_line;
        return;
    }

    w << title_begin << "Net Worth" << title_end;

    std::vector<std::string> columns = {"Name", "Value", "Currency"};
    std::vector<std::vector<std::string>> contents;

    budget::money total;

    for (auto& asset : w.cache.user_assets()) {
        auto amount = get_asset_value(asset, w.cache);

        if (amount) {
            contents.push_back({asset.name, to_string(amount), asset.currency});

            total += amount * exchange_rate(asset.currency);
        }
    }

    contents.push_back({"", "", ""});
    contents.push_back({"Net Worth", budget::to_string(total), get_default_currency()});

    // Display the table

    w.display_table(columns, contents, 1, {}, 1);
}

void budget::show_asset_values(budget::writer& w, bool liability){
    if (no_asset_values() && no_asset_shares()) {
        w << "No asset values" << end_of_line;
        return;
    }

    if (!liability) {
        w << title_begin << "Net Worth" << title_end;

        std::vector<std::string> columns = {"Name"};

        for (auto & clas : w.cache.asset_classes()) {
            columns.emplace_back(clas.name);
        }

        columns.emplace_back("Total");
        columns.emplace_back("Currency");

        std::vector<std::vector<std::string>> contents;

        std::map<std::string, budget::money> classes;

        budget::money assets_total;
        budget::money liabilities_total;

        for(auto& asset : w.cache.user_assets()){
            auto amount = get_asset_value(asset, w.cache);

            if (amount) {
                std::vector<std::string> line;

                line.emplace_back(asset.name);

                for (auto& clas : w.cache.asset_classes()) {
                    bool found = false;

                    for (auto& [class_id, alloc] : asset.classes) {
                        if (class_id == clas.id) {
                            auto class_amount = amount  * (float(alloc) / 100.0);
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

                assets_total += amount * exchange_rate(asset.currency);
            }
        }

        contents.emplace_back(columns.size(), "");

        {
            std::vector<std::string> line;

            line.emplace_back("Total");

            for (auto& clas : w.cache.asset_classes()) {
                if (classes.count(clas.name)) {
                    line.emplace_back(budget::to_string(classes[clas.name]));
                } else {
                    line.emplace_back("0.00");
                }
            }

            line.emplace_back(budget::to_string(assets_total));
            line.emplace_back(budget::get_default_currency());

            contents.emplace_back(std::move(line));
        }

        contents.emplace_back(columns.size(), "");

        {
            std::vector<std::string> line;

            line.emplace_back("Distribution");

            for (auto& clas : w.cache.asset_classes()) {
                if (classes.count(clas.name)) {
                    auto amount = classes[clas.name];
                    line.emplace_back(budget::to_string_precision(100 * amount.dollars() / (double) assets_total.dollars(), 2));
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

            for (auto& clas : w.cache.asset_classes()) {
                auto desired_alloc = get_asset_class_allocation(desired, clas);

                line1.emplace_back(to_string(desired_alloc));
                line2.emplace_back(to_string(assets_total * (float(desired_alloc) / 100.0)));

                if (classes.count(clas.name)) {
                    line3.emplace_back(to_string(assets_total * (float(desired_alloc) / 100.0) - classes[clas.name]));
                } else {
                    line3.emplace_back(to_string(assets_total * (float(desired_alloc) / 100.0)));
                }
            }

            line1.emplace_back("100");
            line1.emplace_back("");

            line2.emplace_back(to_string(assets_total));
            line2.emplace_back(get_default_currency());

            line3.emplace_back("0.00");
            line3.emplace_back(get_default_currency());

            contents.emplace_back(std::move(line1));
            contents.emplace_back(std::move(line2));
            contents.emplace_back(std::move(line3));
        }

        auto liabilities = w.cache.liabilities();

        if (liabilities.size()) {
            contents.emplace_back(columns.size(), "");

            for (auto & liability : liabilities) {
                auto amount = get_liability_value(liability, w.cache);

                if (amount) {
                    std::vector<std::string> line(columns.size(), "");
                    line[0] = liability.name;
                    line[line.size() - 2] = budget::to_string(amount);
                    line[line.size() - 1] = liability.currency;
                    contents.emplace_back(std::move(line));

                    liabilities_total += amount * exchange_rate(liability.currency);
                }
            }
        }

        {
            contents.emplace_back(columns.size(), "");
        }

        {
            std::vector<std::string> line(columns.size(), "");
            line[0] = "Assets";
            line[line.size() - 2] = budget::to_string(assets_total);
            line[line.size() - 1] = get_default_currency();
            contents.emplace_back(std::move(line));

        }

        {
            std::vector<std::string> line(columns.size(), "");
            line[0] = "Liabilities";
            line[line.size() - 2] = budget::to_string(liabilities_total);
            line[line.size() - 1] = get_default_currency();
            contents.emplace_back(std::move(line));

        }

        budget::money net_worth = assets_total - liabilities_total;

        {
            std::vector<std::string> line(columns.size(), "");
            line[0] = "Net Worth";
            line[line.size() - 2] = budget::to_string(net_worth);
            line[line.size() - 1] = get_default_currency();
            contents.emplace_back(std::move(line));

        }

        // Display the table

        if (desired.total_allocation()) {
            w.display_table(columns, contents, 1, {contents.size() - 8, contents.size() - 3}, 1);
        } else {
            w.display_table(columns, contents, 1, {}, 1);
        }
    } else {
        w << title_begin << "Liabilities" << title_end;

        std::vector<std::string> columns = {"Name", "Total", "Currency"};

        std::vector<std::vector<std::string>> contents;

        std::map<std::string, budget::money> classes;

        budget::money total;

        for(auto& asset : w.cache.liabilities()){
            auto amount = get_liability_value(asset, w.cache);

            if (amount) {
                std::vector<std::string> line;

                line.emplace_back(asset.name);
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
            line.emplace_back(budget::to_string(total));
            line.emplace_back(budget::get_default_currency());

            contents.emplace_back(std::move(line));
        }

        // Display the table

        w.display_table(columns, contents, 1, {}, 1);
    }
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

bool budget::no_assets() {
    return assets.empty();
}

void budget::add_asset(budget::asset& asset){
    assets.add(asset);
}

bool budget::edit_asset(const budget::asset& asset){
    return assets.indirect_edit(asset);
}

budget::money budget::get_portfolio_value(){
    budget::money total;

    data_cache cache;

    for (auto & asset : cache.user_assets()) {
        if (asset.portfolio) {
            total += get_asset_value_conv(asset, cache);
        }
    }

    return total;
}

budget::money budget::get_net_worth(data_cache & cache){
    return get_net_worth(budget::local_day(), cache);
}

budget::money budget::get_net_worth(budget::date d, data_cache & cache) {
    budget::money total;

    for (auto & asset : cache.user_assets()) {
        total += get_asset_value_conv(asset, d, cache);
    }

    for (auto & asset : cache.liabilities()) {
        total -= get_liability_value_conv(asset, d, cache);
    }

    return total;
}

budget::money budget::get_net_worth_cash(){
    budget::money total;

    data_cache cache;

    for (auto & asset : cache.user_assets()) {
        if (asset.is_cash()) {
            total += get_asset_value_conv(asset, cache);
        }
    }

    return total;
}

namespace {

int get_shares(const budget::asset& asset, budget::date d, data_cache & cache) {
    int64_t shares = 0;

    for (auto& asset_share : cache.sorted_asset_shares()) {
        if (asset_share.date <= d) {
            if (asset_share.asset_id == asset.id) {
                shares += asset_share.shares;
            }
        } else {
            break;
        }
    }

    return shares;
}

} // namespace

// OPTIM get_asset_value is the current hotspot for almost all pages
// 1) If the share_based part becomes a bottleneck, we can apply the same
//    optimization than for the asset value part
// 2) If this becomes too high, we can also store the value an asset id for each
//    possible date (in one pass of all asset values of an asset)

budget::money budget::get_asset_value(const budget::asset & asset, budget::date d, data_cache & cache) {
    if (cpp_unlikely(asset.share_based)) {
        int64_t shares = get_shares(asset, d, cache);

        if (shares > 0) {
            return static_cast<int>(shares) * share_price(asset.ticker, d);
        }
    } else {
        budget::money asset_value_amount;

        auto & asset_values = cache.sorted_group_asset_values(false)[asset.id];

        if (!asset_values.empty()) {
            auto it = std::upper_bound(asset_values.begin(), asset_values.end(), d, [](budget::date d, auto & value) { return d < value.set_date; });

            if (it != asset_values.begin()) {
                --it;
                asset_value_amount = it->amount;
            }
        }

        return asset_value_amount;
    }

    return {};
}

budget::money budget::get_asset_value(const budget::asset & asset, data_cache & cache) {
    return get_asset_value(asset, budget::local_day(), cache);
}

budget::money budget::get_asset_value_conv(const budget::asset & asset, data_cache & cache) {
    return get_asset_value_conv(asset, budget::local_day(), cache);
}

budget::money budget::get_asset_value_conv(const budget::asset & asset, budget::date d, data_cache & cache) {
    auto amount = get_asset_value(asset, d, cache);

    if (amount) {
        return amount * exchange_rate(asset.currency, d);
    } else {
        return amount;
    }
}

budget::money budget::get_asset_value_conv(const budget::asset & asset, const std::string& currency, data_cache & cache) {
    return get_asset_value_conv(asset, budget::local_day(), currency, cache);
}

budget::money budget::get_asset_value_conv(const budget::asset & asset, budget::date d, const std::string& currency, data_cache & cache) {
    auto amount = get_asset_value(asset, d, cache);

    if (amount) {
        return amount * exchange_rate(asset.currency, currency, d);
    } else {
        return amount;
    }
}

bool budget::is_ticker_active(data_cache & cache, const std::string & ticker) {
    for (auto & asset : cache.assets()) {
        if (asset.share_based) {
            if (asset.ticker == ticker) {
                int64_t shares = get_shares(asset, local_day(), cache);

                if (shares > 0) {
                    return true;
                }
            }
        }
    }

    return false;
}
