//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

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

using namespace budget;

namespace {

static data_handler<asset> assets;

void show_assets(){
    if (!assets.data.size()) {
        std::cout << "No assets" << std::endl;
        return;
    }

    std::vector<std::string> columns = {"ID", "Name", "Int. Stocks", "Swiss Stocks", "Bonds", "Cash", "Currency"};
    std::vector<std::vector<std::string>> contents;

    // Display the assets

    for(auto& asset : assets.data){
        contents.push_back({to_string(asset.id), asset.name, to_string(asset.int_stocks),
            to_string(asset.swiss_stocks), to_string(asset.bonds), to_string(asset.cash), to_string(asset.currency)});
    }

    display_table(columns, contents);
}

} //end of anonymous namespace

void budget::assets_module::load(){
    load_assets();
}

void budget::assets_module::unload(){
    save_assets();
}

void budget::assets_module::handle(const std::vector<std::string>& args){
    if(args.size() == 1){
        show_assets();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "show"){
            show_assets();
        } else if(subcommand == "add"){
            asset asset;
            asset.guid = generate_guid();

            edit_string(asset.name, "Name", not_empty_checker());

            if(asset_exists(asset.name)){
                throw budget_exception("An asset with this name already exists");
            }

            asset.int_stocks = 0;
            asset.swiss_stocks = 0;
            asset.bonds = 0;
            asset.cash = 0;

            do {
                edit_number(asset.int_stocks, "Int. Stocks");
                edit_number(asset.swiss_stocks, "Swiss Stocks");
                edit_number(asset.bonds, "Bonds");
                edit_number(asset.cash, "Cash");

                if(asset.int_stocks + asset.swiss_stocks + asset.bonds + asset.cash != 100){
                    std::cout << "The distribution must account to 100%" << std::endl;
                }
            } while (asset.int_stocks + asset.swiss_stocks + asset.bonds + asset.cash != 100);

            asset.currency = "CHF";
            edit_string(asset.currency, "Currency", not_empty_checker());

            auto id = add_data(assets, std::move(asset));
            std::cout << "Asset " << id << " has been created" << std::endl;
        } else if(subcommand == "delete"){
            size_t id = 0;

            if(args.size() >= 3){
                enough_args(args, 3);

                id = to_number<size_t>(args[2]);

                if (!exists(assets, id)) {
                    throw budget_exception("There are no asset with id " + args[2]);
                }
            } else {
                std::string name;
                edit_string(name, "Asset", not_empty_checker(), asset_checker());

                id = budget::get_asset(name).id;
            }

            // TODO Make sure there is no values to this asset

            remove(assets, id);

            std::cout << "Asset " << id << " has been deleted" << std::endl;
        } else if(subcommand == "edit"){
            size_t id = 0;

            if(args.size() >= 3){
                enough_args(args, 3);

                id = to_number<size_t>(args[2]);

                if(!exists(assets, id)){
                    throw budget_exception("There are no asset with id " + args[2]);
                }
            } else {
                std::string name;
                edit_string(name, "Asset", not_empty_checker(), asset_checker());

                id = get_asset(name).id;
            }

            auto& asset = get(assets, id);

            edit_string(asset.name, "Name", not_empty_checker());

            //Verify that there are no OTHER asset with this name
            //in the current set of assets (taking archiving into asset)

            for(auto& other_asset : all_assets()){
                if(other_asset.id != id){
                    if(other_asset.name == asset.name){
                        throw budget_exception("There is already an asset with the name " + asset.name);
                    }
                }
            }

            do {
                edit_number(asset.int_stocks, "Int. Stocks");
                edit_number(asset.swiss_stocks, "Swiss Stocks");
                edit_number(asset.bonds, "Bonds");
                edit_number(asset.cash, "Cash");

                if(asset.int_stocks + asset.swiss_stocks + asset.bonds + asset.cash != 100){
                    std::cout << "The distribution must asset to 100%" << std::endl;
                }
            } while (asset.int_stocks + asset.swiss_stocks + asset.bonds + asset.cash != 100);

            edit_string(asset.currency, "Currency", not_empty_checker());

            std::cout << "Asset " << id << " has been modified" << std::endl;

            assets.changed = true;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_assets(){
    load_data(assets, "assets.data");
}

void budget::save_assets(){
    save_data(assets, "assets.data");
}

budget::asset& budget::get_asset(size_t id){
    return get(assets, id);
}

budget::asset& budget::get_asset(std::string name){
    for(auto& asset : assets.data){
        if(asset.name == name){
            return asset;
        }
    }

    cpp_unreachable("The asset does not exist");
}

std::ostream& budget::operator<<(std::ostream& stream, const asset& asset){
    return stream << asset.id << ':' << asset.guid << ':' << asset.name << ':'
        << asset.int_stocks << ':' << asset.swiss_stocks << ":" << asset.bonds << ":" << asset.cash << ":" << asset.currency;
}

void budget::operator>>(const std::vector<std::string>& parts, asset& asset){
    asset.id = to_number<size_t>(parts[0]);
    asset.guid = parts[1];
    asset.name = parts[2];
    asset.int_stocks = to_number<size_t>(parts[3]);
    asset.swiss_stocks = to_number<size_t>(parts[4]);
    asset.bonds = to_number<size_t>(parts[5]);
    asset.cash = to_number<size_t>(parts[6]);
    asset.currency = parts[7];
}

bool budget::asset_exists(const std::string& name){
    for(auto& asset : assets.data){
        if(asset.name == name){
            return true;
        }
    }

    return false;
}

std::vector<asset>& budget::all_assets(){
    return assets.data;
}

void budget::set_assets_changed(){
    assets.changed = true;
}

void budget::set_assets_next_id(size_t next_id){
    assets.next_id = next_id;
}
