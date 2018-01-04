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
#include "writer.hpp"

#include <curl/curl.h>

using namespace budget;

namespace {

static data_handler<asset> assets { "assets", "assets.data" };
static data_handler<asset_value> asset_values { "asset_values", "asset_values.data" };

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

} //end of anonymous namespace

std::map<std::string, std::string> budget::asset::get_params(){
    std::map<std::string, std::string> params;

    params["input_id"]              = budget::to_string(id);
    params["input_guid"]            = guid;
    params["input_name"]            = name;
    params["input_int_stocks"]      = budget::to_string(int_stocks);
    params["input_dom_stocks"]      = budget::to_string(dom_stocks);
    params["input_bonds"]           = budget::to_string(bonds);
    params["input_cash"]            = budget::to_string(cash);
    params["input_currency"]        = currency;
    params["input_portfolio"]       = portfolio ? "true" : "false";
    params["input_portfolio_alloc"] = budget::to_string(portfolio_alloc);

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

            asset.int_stocks      = 0;
            asset.dom_stocks      = 0;
            asset.bonds           = 0;
            asset.cash            = 0;
            asset.portfolio       = false;
            asset.portfolio_alloc = 0;

            do {
                edit_money(asset.int_stocks, "Int. Stocks");
                edit_money(asset.dom_stocks, "Dom. Stocks");
                edit_money(asset.bonds, "Bonds");
                edit_money(asset.cash, "Cash");

                if(asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != money(100)){
                    std::cout << "The distribution must account to 100%" << std::endl;
                }
            } while (asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != money(100));

            asset.currency = budget::get_default_currency();
            edit_string(asset.currency, "Currency", not_empty_checker());

            std::cout << "Is this part of your portfolio ? [yes/no] ? ";

            std::string answer;
            std::getline(std::cin, answer);

            if (answer == "yes" || answer == "y") {
                asset.portfolio = true;
            }

            if (asset.portfolio) {
                edit_money(asset.portfolio_alloc, "Portfolio Allocation");
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

            for(auto& value : asset_values.data){
                if(value.asset_id == id){
                    throw budget_exception("There are still asset values linked to asset " + args[2]);
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

            for(auto& other_asset : all_assets()){
                if(other_asset.id != id){
                    if(other_asset.name == asset.name){
                        throw budget_exception("There is already an asset with the name " + asset.name);
                    }
                }
            }

            do {
                edit_money(asset.int_stocks, "Int. Stocks");
                edit_money(asset.dom_stocks, "Dom. Stocks");
                edit_money(asset.bonds, "Bonds");
                edit_money(asset.cash, "Cash");

                if(asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != money(100)){
                    std::cout << "The distribution must asset to 100%" << std::endl;
                }
            } while (asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != money(100));

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
                edit_money(asset.portfolio_alloc, "Portfolio Allocation");
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
        } else if(subcommand == "distribution"){
            auto& desired = get_desired_allocation();

            do {
                edit_money(desired.int_stocks, "Int. Stocks");
                edit_money(desired.dom_stocks, "Dom. Stocks");
                edit_money(desired.bonds, "Bonds");
                edit_money(desired.cash, "Cash");

                if(desired.int_stocks + desired.dom_stocks + desired.bonds + desired.cash != money(100)){
                    std::cout << "The distribution must account to 100%" << std::endl;
                }
            } while (desired.int_stocks + desired.dom_stocks + desired.bonds + desired.cash != money(100));

            if (assets.edit(desired)) {
                std::cout << "The distribution has been modified" << std::endl;
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::load_assets(){
    assets.load();
    asset_values.load();
}

void budget::save_assets(){
    assets.save();
    asset_values.save();
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

    auto id = assets.add(std::move(asset));
    return get_asset(id);
}

budget::asset_value& budget::get_asset_value(size_t id){
    return asset_values[id];
}

std::ostream& budget::operator<<(std::ostream& stream, const asset& asset){
    return stream << asset.id << ':' << asset.guid << ':' << asset.name << ':'
        << asset.int_stocks << ':' << asset.dom_stocks << ":" << asset.bonds << ":" << asset.cash << ":" << asset.currency << ":" << asset.portfolio << ":" << asset.portfolio_alloc;
}

void budget::operator>>(const std::vector<std::string>& parts, asset& asset){
    bool random = config_contains("random");

    asset.id              = to_number<size_t>(parts[0]);
    asset.guid            = parts[1];
    asset.int_stocks      = parse_money(parts[3]);
    asset.dom_stocks      = parse_money(parts[4]);
    asset.bonds           = parse_money(parts[5]);
    asset.cash            = parse_money(parts[6]);
    asset.currency        = parts[7];
    asset.portfolio       = to_number<size_t>(parts[8]);
    asset.portfolio_alloc = parse_money(parts[9]);

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
    assets.set_changed();
}

void budget::set_asset_values_changed(){
    asset_values.set_changed();
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

    w << title_begin << "Assets" << title_end;

    std::vector<std::string> columns = {"ID", "Name", "Int. Stocks", "Dom. Stocks", "Bonds", "Cash", "Currency", "Portfolio", "Alloc", "Edit"};
    std::vector<std::vector<std::string>> contents;

    // Display the assets

    for(auto& asset : assets.data){
        if(asset.name == "DESIRED" && asset.currency == "DESIRED"){
            continue;
        }

        contents.push_back({to_string(asset.id), asset.name, to_string(asset.int_stocks),
            to_string(asset.dom_stocks), to_string(asset.bonds), to_string(asset.cash),
            to_string(asset.currency), asset.portfolio ? "Yes" : "No", asset.portfolio ? to_string(asset.portfolio_alloc) : "", "::edit::assets::" + budget::to_string(asset.id)});
    }

    w.display_table(columns, contents);
}


void budget::show_asset_portfolio(budget::writer& w){
    if (!asset_values.data.size()) {
        w << "No asset values" << end_of_line;
        return;
    }

    w << title_begin << "Portfolio" << title_end;

    std::vector<std::string> columns = {"Name", "Total", "Currency", "Converted", "Allocation"};
    std::vector<std::vector<std::string>> contents;

    budget::money total;

    for(auto& asset : assets.data){
        auto id = asset.id;

        if (asset.portfolio) {
            size_t asset_value_id  = 0;
            bool asset_value_found = false;

            for (auto& asset_value : asset_values.data) {
                if (asset_value.asset_id == id) {
                    if (!asset_value_found) {
                        asset_value_found = true;
                        asset_value_id    = asset_value.id;
                    } else if (asset_value.set_date >= get_asset_value(asset_value_id).set_date) {
                        asset_value_id = asset_value.id;
                    }
                }
            }

            if (asset_value_found) {
                auto& asset_value = get_asset_value(asset_value_id);

                auto conv_amount = asset_value.amount * exchange_rate(asset.currency, get_default_currency());

                total += conv_amount;
            }
        }
    }

    for(auto& asset : assets.data){
        auto id = asset.id;

        if (asset.portfolio) {
            size_t asset_value_id  = 0;
            bool asset_value_found = false;

            for (auto& asset_value : asset_values.data) {
                if (asset_value.asset_id == id) {
                    if (!asset_value_found) {
                        asset_value_found = true;
                        asset_value_id    = asset_value.id;
                    } else if (asset_value.set_date >= get_asset_value(asset_value_id).set_date) {
                        asset_value_id = asset_value.id;
                    }
                }
            }

            if (asset_value_found) {
                auto& asset_value = get_asset_value(asset_value_id);
                auto amount       = asset_value.amount;
                auto conv_amount  = asset_value.amount * exchange_rate(asset.currency, get_default_currency());
                auto allocation   = 100.0 * (conv_amount / total);

                if (amount) {
                    contents.push_back({
                        asset.name,
                        to_string(amount),
                        asset.currency,
                        to_string(conv_amount),
                        to_percent(allocation)
                    });
                }
            }
        }
    }

    w.display_table(columns, contents, 1, {}, 1);

    std::vector<std::string> second_columns;
    std::vector<std::vector<std::string>> second_contents;

    second_contents.emplace_back(std::vector<std::string>{"Total", budget::to_string(total) + get_default_currency()});

    w.display_table(second_columns, second_contents, 1, {}, 15);
}

void budget::show_asset_rebalance(budget::writer& w){
    if (!asset_values.data.size()) {
        w << "No asset values" << end_of_line;
        return;
    }

    w << title_begin << "Rebalancing" << title_end;

    std::vector<std::string> columns = {"Name", "Total", "Currency", "Converted", "Allocation", "Desired Allocation", "Desired Total", "Difference"};
    std::vector<std::vector<std::string>> contents;

    budget::money total;

    for(auto& asset : assets.data){
        auto id = asset.id;

        if (asset.portfolio) {
            size_t asset_value_id  = 0;
            bool asset_value_found = false;

            for (auto& asset_value : asset_values.data) {
                if (asset_value.asset_id == id) {
                    if (!asset_value_found) {
                        asset_value_found = true;
                        asset_value_id    = asset_value.id;
                    } else if (asset_value.set_date >= get_asset_value(asset_value_id).set_date) {
                        asset_value_id = asset_value.id;
                    }
                }
            }

            if (asset_value_found) {
                auto& asset_value = get_asset_value(asset_value_id);

                auto conv_amount = asset_value.amount * exchange_rate(asset.currency, get_default_currency());

                total += conv_amount;
            }
        }
    }

    budget::money total_rebalance;

    for(auto& asset : assets.data){
        auto id = asset.id;

        if (asset.portfolio) {
            size_t asset_value_id  = 0;
            bool asset_value_found = false;

            for (auto& asset_value : asset_values.data) {
                if (asset_value.asset_id == id) {
                    if (!asset_value_found) {
                        asset_value_found = true;
                        asset_value_id    = asset_value.id;
                    } else if (asset_value.set_date >= get_asset_value(asset_value_id).set_date) {
                        asset_value_id = asset_value.id;
                    }
                }
            }

            if (asset_value_found) {
                auto& asset_value = get_asset_value(asset_value_id);
                auto amount       = asset_value.amount;

                auto conv_amount = amount * exchange_rate(asset.currency, get_default_currency());
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
    }

    // Display the total rebalancing effort
    contents.push_back({"", "", "", "", "", "", "", ""});
    contents.push_back({"Total effort", "", "", "", "", "", "", format_money(total_rebalance)});

    w.display_table(columns, contents, 1, {}, 1);
}

void budget::show_asset_values(budget::writer& w){
    if (!asset_values.data.size()) {
        w << "No asset values" << end_of_line;
        return;
    }

    w << title_begin << "Net Worth" << title_end;

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
                } else if(asset_value.set_date >= get_asset_value(asset_value_id).set_date){
                    asset_value_id    = asset_value.id;
                }
            }
        }

        if(asset_value_found){
            auto& asset_value = get_asset_value(asset_value_id);
            auto amount       = asset_value.amount;

            if (amount) {
                contents.push_back({asset.name,
                                    to_string(amount * (float(asset.int_stocks) / 100.0)),
                                    to_string(amount * (float(asset.dom_stocks) / 100.0)),
                                    to_string(amount * (float(asset.bonds) / 100.0)),
                                    to_string(amount * (float(asset.cash) / 100.0)),
                                    to_string(amount),
                                    asset.currency});

                auto int_stocks_amount = amount * (float(asset.int_stocks) / 100.0);
                auto dom_stocks_amount = amount * (float(asset.dom_stocks) / 100.0);
                auto bonds_amount      = amount * (float(asset.bonds) / 100.0);
                auto cash_amount       = amount * (float(asset.cash) / 100.0);

                int_stocks += int_stocks_amount * exchange_rate(asset.currency, get_default_currency());
                dom_stocks += dom_stocks_amount * exchange_rate(asset.currency, get_default_currency());
                bonds += bonds_amount * exchange_rate(asset.currency, get_default_currency());
                cash += cash_amount * exchange_rate(asset.currency, get_default_currency());
                total += amount * exchange_rate(asset.currency, get_default_currency());
            }
        }
    }

    contents.emplace_back(columns.size(), "");

    contents.push_back({"Total",
                        to_string(int_stocks),
                        to_string(dom_stocks),
                        to_string(bonds),
                        to_string(cash),
                        to_string(total),
                        budget::get_default_currency()});

    contents.emplace_back(columns.size(), "");

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
                            to_string(total * (float(desired.int_stocks) / 100.0)),
                            to_string(total * (float(desired.dom_stocks) / 100.0)),
                            to_string(total * (float(desired.bonds) / 100.0)),
                            to_string(total * (float(desired.cash) / 100.0)),
                            to_string(total),
                            get_default_currency()});

        contents.push_back({"Difference (need)",
                            to_string(total * (float(desired.int_stocks) / 100.0) - int_stocks),
                            to_string(total * (float(desired.dom_stocks) / 100.0) - dom_stocks),
                            to_string(total * (float(desired.bonds) / 100.0) - bonds),
                            to_string(total * (float(desired.cash) / 100.0) - cash),
                            to_string(budget::money{}),
                            get_default_currency()});
    }

    contents.push_back({"", "", "", "", "", "", ""});
    contents.push_back({"Net Worth", "", "", "", "", budget::to_string(total), get_default_currency()});

    // Display the table

    if (desired.total_allocation()) {
        w.display_table(columns, contents, 1, {contents.size() - 8, contents.size() - 3}, 1);
    } else {
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
