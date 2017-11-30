//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <map>

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

#include <curl/curl.h>

using namespace budget;

namespace {

static data_handler<asset> assets;
static data_handler<asset_value> asset_values;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::map<std::pair<std::string, std::string>, double> exchanges;

double exchange_rate(std::string from, std::string to){
    if(from == to){
        return 1.0;
    } else {
        auto key = std::make_pair(from, to);

        if (!exchanges.count(key)) {
            auto url = "http://free.currencyconverterapi.com/api/v3/convert?q=" + from + "_" + to + "&compact=ultra";
            CURL* curl;
            CURLcode res;
            std::string buffer;

            curl = curl_easy_init();
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                if (res) {
                    std::cout << "Error accessing exchange rates, setting exchange between " << from << " to " << to << " to 1/1" << std::endl;

                    exchanges[key] = 1.0;
                } else {
                    if (buffer.find(':') == std::string::npos || buffer.find('}') == std::string::npos) {
                        std::cout << "Error parsing exchange rates, setting exchange between " << from << " to " << to << " to 1/1" << std::endl;

                        exchanges[key] = 1.0;
                    } else {
                        std::string ratio_result(buffer.begin() + buffer.find(':') + 1, buffer.begin() + buffer.find('}'));

                        exchanges[key] = atof(ratio_result.c_str());
                    }
                }
            }
        }

        return exchanges[key];
    }
}

std::vector<std::string> get_asset_names(){
    std::vector<std::string> asset_names;

    for (auto& asset : all_assets()) {
        if (asset.name != "DESIRED") {
            asset_names.push_back(asset.name);
        }
    }

    return asset_names;
}

void show_assets(){
    if (!assets.data.size()) {
        std::cout << "No assets" << std::endl;
        return;
    }

    std::vector<std::string> columns = {"ID", "Name", "Int. Stocks", "Dom. Stocks", "Bonds", "Cash", "Currency"};
    std::vector<std::vector<std::string>> contents;

    // Display the assets

    for(auto& asset : assets.data){
        if(asset.name == "DESIRED" && asset.currency == "DESIRED"){
            continue;
        }

        contents.push_back({to_string(asset.id), asset.name, to_string(asset.int_stocks),
            to_string(asset.dom_stocks), to_string(asset.bonds), to_string(asset.cash), to_string(asset.currency)});
    }

    display_table(columns, contents);
}

void list_asset_values(){
    if (!asset_values.data.size()) {
        std::cout << "No asset values" << std::endl;
        return;
    }

    std::vector<std::string> columns = {"ID", "Asset", "Amount", "Date"};
    std::vector<std::vector<std::string>> contents;

    // Display the assets

    for(auto& value : asset_values.data){
        contents.push_back({to_string(value.id), get_asset(value.asset_id).name, to_string(value.amount), to_string(value.set_date)});
    }

    display_table(columns, contents);
}

void show_asset_values(){
    if (!asset_values.data.size()) {
        std::cout << "No asset values" << std::endl;
        return;
    }

    std::vector<std::string> columns = {"Name", "Int. Stocks", "Dom. Stocks", "Bonds", "Cash", "Total", "Currency"};
    std::vector<std::vector<std::string>> contents;

    budget::money int_stocks;
    budget::money dom_stocks;
    budget::money bonds;
    budget::money cash;
    budget::money total;

    for(auto& asset : assets.data){
        auto id = asset.id;

        size_t asset_value_id = 0;
        bool asset_value_found = false;

        for (auto& asset_value : asset_values.data) {
            if (asset_value.asset_id == id) {
                if(!asset_value_found){
                    asset_value_found = true;
                    asset_value_id    = asset_value.id;
                } else if(asset_value.set_date > get_asset_value(asset_value_id).set_date){
                    asset_value_id    = asset_value.id;
                }
            }
        }

        if(asset_value_found){
            auto& asset_value = get_asset_value(asset_value_id);
            auto amount       = asset_value.amount;

            contents.push_back({asset.name,
                                to_string(amount * (asset.int_stocks / 100.0)),
                                to_string(amount * (asset.dom_stocks / 100.0)),
                                to_string(amount * (asset.bonds / 100.0)),
                                to_string(amount * (asset.cash / 100.0)),
                                to_string(amount),
                                asset.currency});

            auto int_stocks_amount = amount * (asset.int_stocks / 100.0);
            auto dom_stocks_amount = amount * (asset.dom_stocks / 100.0);
            auto bonds_amount      = amount * (asset.bonds / 100.0);
            auto cash_amount       = amount * (asset.cash / 100.0);

            int_stocks += int_stocks_amount * exchange_rate(asset.currency, get_default_currency());
            dom_stocks += dom_stocks_amount * exchange_rate(asset.currency, get_default_currency());
            bonds += bonds_amount * exchange_rate(asset.currency, get_default_currency());
            cash += cash_amount * exchange_rate(asset.currency, get_default_currency());
            total += amount * exchange_rate(asset.currency, get_default_currency());
        }
    }

    contents.push_back({"Total",
                        to_string(int_stocks),
                        to_string(dom_stocks),
                        to_string(bonds),
                        to_string(cash),
                        to_string(total),
                        budget::get_default_currency()});

    contents.push_back({"Distribution",
                        to_string_precision(100 * int_stocks.dollars() / (double)total.dollars(), 2),
                        to_string_precision(100 * dom_stocks.dollars() / (double)total.dollars(), 2),
                        to_string_precision(100 * bonds.dollars() / (double)total.dollars(), 2),
                        to_string_precision(100 * cash.dollars() / (double)total.dollars(), 2),
                        to_string(100),
                        ""});

    decltype(auto) desired = get_desired_allocation();

    if (desired.total_allocation()) {
        contents.push_back({"Desired Distribution",
                            to_string(desired.int_stocks),
                            to_string(desired.dom_stocks),
                            to_string(desired.bonds),
                            to_string(desired.cash),
                            to_string(100),
                            ""});

        contents.push_back({"Desired Total",
                            to_string(total * (desired.int_stocks / 100.0)),
                            to_string(total * (desired.dom_stocks / 100.0)),
                            to_string(total * (desired.bonds / 100.0)),
                            to_string(total * (desired.cash / 100.0)),
                            to_string(total),
                            get_default_currency()});

        contents.push_back({"Difference (need)",
                            to_string(total * (desired.int_stocks / 100.0) - int_stocks),
                            to_string(total * (desired.dom_stocks / 100.0) - dom_stocks),
                            to_string(total * (desired.bonds / 100.0) - bonds),
                            to_string(total * (desired.cash / 100.0) - cash),
                            to_string(budget::money{}),
                            get_default_currency()});

        display_table(columns, contents, 1, {contents.size() - 5, contents.size() - 1}, 1);
    } else {
        display_table(columns, contents, 1, {}, 1);
    }

    std::cout << std::endl << "       Net worth: " << total << get_default_currency() << std::endl;
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

            asset.int_stocks      = 0;
            asset.dom_stocks      = 0;
            asset.bonds           = 0;
            asset.cash            = 0;
            asset.portfolio       = false;
            asset.portfolio_alloc = 0;

            do {
                edit_number(asset.int_stocks, "Int. Stocks");
                edit_number(asset.dom_stocks, "Dom. Stocks");
                edit_number(asset.bonds, "Bonds");
                edit_number(asset.cash, "Cash");

                if(asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != 100){
                    std::cout << "The distribution must account to 100%" << std::endl;
                }
            } while (asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != 100);

            asset.currency = budget::get_default_currency();
            edit_string(asset.currency, "Currency", not_empty_checker());

            std::cout << "Is this part of your portfolio ? [yes/no] ? ";

            std::string answer;
            std::getline(std::cin, answer);

            if (answer == "yes" || answer == "y") {
                asset.portfolio = true;
            }

            if (asset.portfolio) {
                edit_number(asset.portfolio_alloc, "Portfolio Allocation");
            }

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
                edit_string_complete(name, "Asset", get_asset_names(), not_empty_checker(), asset_checker());

                id = budget::get_asset(name).id;
            }

            auto& asset = get_asset(id);

            if(asset.name == "DESIRED" && asset.currency == "DESIRED"){
                throw budget_exception("Cannot delete special asset " + args[2]);
            }

            for(auto& value : asset_values.data){
                if(value.asset_id == id){
                    throw budget_exception("There are still asset values linked to asset " + args[2]);
                }
            }

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
                edit_string_complete(name, "Asset", get_asset_names(), not_empty_checker(), asset_checker());

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
                edit_number(asset.dom_stocks, "Dom. Stocks");
                edit_number(asset.bonds, "Bonds");
                edit_number(asset.cash, "Cash");

                if(asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != 100){
                    std::cout << "The distribution must asset to 100%" << std::endl;
                }
            } while (asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != 100);

            edit_string(asset.currency, "Currency", not_empty_checker());

            std::cout << "Is this part of your portfolio ? [yes/no] ? ";

            std::string answer;
            std::getline(std::cin, answer);

            if (answer == "yes" || answer == "y") {
                asset.portfolio = true;
            } else {
                asset.portfolio = false;
            }

            if (asset.portfolio) {
                edit_number(asset.portfolio_alloc, "Portfolio Allocation");
            } else {
                asset.portfolio_alloc = 0;
            }

            std::cout << "Asset " << id << " has been modified" << std::endl;

            assets.changed = true;
        } else if(subcommand == "value"){
            if(args.size() == 2){
                show_asset_values();
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

                    auto id = add_data(asset_values, std::move(asset_value));
                    std::cout << "Asset Value " << id << " has been created" << std::endl;
                } else if(subsubcommand == "show"){
                    show_asset_values();
                } else if(subsubcommand == "list"){
                    list_asset_values();
                } else if(subsubcommand == "edit"){
                    size_t id = 0;

                    enough_args(args, 4);

                    id = to_number<size_t>(args[3]);

                    if (!exists(assets, id)) {
                        throw budget_exception("There are no asset values with id " + args[2]);
                    }

                    auto& value = get(asset_values, id);

                    std::string asset_name = get_asset(value.asset_id).name;
                    edit_string_complete(asset_name, "Asset", get_asset_names(), not_empty_checker(), asset_checker());
                    value.asset_id = get_asset(asset_name).id;

                    edit_money(value.amount, "Amount", not_negative_checker());
                    edit_date(value.set_date, "Date");

                    std::cout << "Asset Value " << id << " has been modified" << std::endl;

                    asset_values.changed = true;
                } else if (subsubcommand == "delete") {
                    size_t id = 0;

                    enough_args(args, 4);

                    id = to_number<size_t>(args[3]);

                    if (!exists(assets, id)) {
                        throw budget_exception("There are no asset value with id " + args[2]);
                    }

                    remove(asset_values, id);

                    std::cout << "Asset value " << id << " has been deleted" << std::endl;
                } else {
                    throw budget_exception("Invalid subcommand value \"" + subsubcommand + "\"");
                }
            }
        } else if(subcommand == "distribution"){
            auto& desired = get_desired_allocation();

            do {
                edit_number(desired.int_stocks, "Int. Stocks");
                edit_number(desired.dom_stocks, "Dom. Stocks");
                edit_number(desired.bonds, "Bonds");
                edit_number(desired.cash, "Cash");

                if(desired.int_stocks + desired.dom_stocks + desired.bonds + desired.cash != 100){
                    std::cout << "The distribution must account to 100%" << std::endl;
                }
            } while (desired.int_stocks + desired.dom_stocks + desired.bonds + desired.cash != 100);

            assets.changed = true;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_assets(){
    load_data(assets, "assets.data");
    load_data(asset_values, "asset_values.data");
}

void budget::save_assets(){
    save_data(assets, "assets.data");
    save_data(asset_values, "asset_values.data");
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

budget::asset& budget::get_desired_allocation(){
    for(auto& asset : assets.data){
        if(asset.name == "DESIRED" && asset.currency == "DESIRED"){
            return asset;
        }
    }

    asset asset;
    asset.guid       = generate_guid();
    asset.name       = "DESIRED";
    asset.currency   = "DESIRED";
    asset.int_stocks = 0;
    asset.dom_stocks = 0;
    asset.bonds      = 0;
    asset.cash       = 0;

    auto id = add_data(assets, std::move(asset));
    return get_asset(id);
}

budget::asset_value& budget::get_asset_value(size_t id){
    return get(asset_values, id);
}

std::ostream& budget::operator<<(std::ostream& stream, const asset& asset){
    return stream << asset.id << ':' << asset.guid << ':' << asset.name << ':'
        << asset.int_stocks << ':' << asset.dom_stocks << ":" << asset.bonds << ":" << asset.cash << ":" << asset.currency << ":" << asset.portfolio << ":" << asset.portfolio_alloc;
}

void budget::operator>>(const std::vector<std::string>& parts, asset& asset){
    bool random = config_contains("random");

    asset.id              = to_number<size_t>(parts[0]);
    asset.guid            = parts[1];
    asset.int_stocks      = to_number<size_t>(parts[3]);
    asset.dom_stocks      = to_number<size_t>(parts[4]);
    asset.bonds           = to_number<size_t>(parts[5]);
    asset.cash            = to_number<size_t>(parts[6]);
    asset.currency        = parts[7];
    asset.portfolio       = to_number<size_t>(parts[8]);
    asset.portfolio_alloc = to_number<size_t>(parts[9]);

    if (random) {
        asset.name = parts[2];

        if (!(asset.name == "DESIRED" && asset.currency == "DESIRED")) {
            asset.name = budget::random_name(5);
        }
    } else {
        asset.name = parts[2];
    }
}

std::ostream& budget::operator<<(std::ostream& stream, const asset_value& asset_value){
    return stream << asset_value.id << ':' << asset_value.guid << ':' << asset_value.asset_id << ":" << asset_value.amount << ":" << to_string(asset_value.set_date);
}

void budget::operator>>(const std::vector<std::string>& parts, asset_value& asset_value){
    bool random = config_contains("random");

    asset_value.id       = to_number<size_t>(parts[0]);
    asset_value.guid     = parts[1];
    asset_value.asset_id = to_number<size_t>(parts[2]);
    asset_value.set_date = from_string(parts[4]);

    if (random) {
        asset_value.amount = budget::random_money(1000, 50000);
    } else {
        asset_value.amount = parse_money(parts[3]);
    }
}

bool budget::asset_exists(const std::string& name){
    for (auto& asset : assets.data) {
        if (asset.name == name) {
            return true;
        }
    }

    return false;
}

std::vector<asset>& budget::all_assets(){
    return assets.data;
}

std::vector<asset_value>& budget::all_asset_values(){
    return asset_values.data;
}

void budget::set_assets_changed(){
    assets.changed = true;
}

void budget::set_asset_values_changed(){
    asset_values.changed = true;
}

void budget::set_assets_next_id(size_t next_id){
    assets.next_id = next_id;
}

void budget::set_asset_values_next_id(size_t next_id){
    asset_values.next_id = next_id;
}

std::string budget::get_default_currency(){
    if(budget::config_contains("default_currency")){
        return budget::config_value("default_currency");
    }

    return "CHF";
}
