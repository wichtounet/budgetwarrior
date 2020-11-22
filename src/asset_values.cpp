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

#include "assets.hpp"
#include "liabilities.hpp"
#include "budget_exception.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

static data_handler<asset_value> asset_values { "asset_values", "asset_values.data" };

} //end of anonymous namespace

std::map<std::string, std::string> budget::asset_value::get_params() const {
    std::map<std::string, std::string> params;

    params["input_id"]       = budget::to_string(id);
    params["input_guid"]     = guid;
    params["input_asset_id"] = budget::to_string(asset_id);
    params["input_amount"]   = budget::to_string(amount);
    params["input_set_date"] = budget::to_string(set_date);
    params["input_liability"] = liability ? "true" : "false";

    return params;
}

void budget::load_asset_values(){
    asset_values.load();
}

void budget::save_asset_values(){
    asset_values.save();
}

budget::asset_value budget::get_asset_value(size_t id){
    return asset_values[id];
}

void budget::asset_value::save(data_writer & writer){
    writer << id;
    writer << guid;
    writer << asset_id;
    writer << amount;
    writer << set_date;
    writer << liability;
}

void budget::asset_value::load(data_reader & reader){
    reader >> id;
    reader >> guid;
    reader >> asset_id;
    reader >> amount;
    reader >> set_date;

    if (reader.more()) {
        reader >> liability;
    } else {
        liability = false;
    }

    if (guid == "XXXXX") {
        guid = generate_guid();
    }

    if (config_contains("random")) {
        amount = budget::random_money(1000, 50000);
    }
}

std::vector<asset_value> budget::all_asset_values(){
    return asset_values.data();
}

void budget::set_asset_values_changed(){
    asset_values.set_changed();
}

void budget::set_asset_values_next_id(size_t next_id){
    asset_values.next_id = next_id;
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

bool budget::no_asset_values() {
    return asset_values.empty();
}

size_t budget::add_asset_value(budget::asset_value& asset_value){
    return asset_values.add(asset_value);
}

bool budget::edit_asset_value(asset_value& asset_value) {
    return asset_values.indirect_edit(asset_value);
}

void budget::list_asset_values(budget::writer& w, bool liability){
    if (!asset_values.size()) {
        w << "No asset values" << end_of_line;
        return;
    }

    std::vector<std::string> columns = {"ID", "Asset", "Amount", "Date", "Edit"};
    std::vector<std::vector<std::string>> contents;

    // Display the asset values

    for(auto& value : asset_values.data()){
        if (value.liability == liability) {
            if (liability) {
                contents.push_back({to_string(value.id), get_liability(value.asset_id).name, to_string(value.amount), to_string(value.set_date), "::edit::asset_values::" + budget::to_string(value.id)});
            } else {
                contents.push_back({to_string(value.id), get_asset(value.asset_id).name, to_string(value.amount), to_string(value.set_date), "::edit::asset_values::" + budget::to_string(value.id)});
            }
        }
    }

    w.display_table(columns, contents);
}
