//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <random>

#include "assets.hpp"
#include "budget_exception.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

static data_handler<asset_share> asset_shares { "asset_shares", "asset_shares.data" };

} //end of anonymous namespace

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

void budget::load_asset_shares() {
    asset_shares.load();
}

void budget::save_asset_shares() {
    asset_shares.save();
}

budget::asset_share& budget::get_asset_share(size_t id) {
    return asset_shares[id];
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

std::vector<asset_share>& budget::all_asset_shares(){
    return asset_shares.data;
}

void budget::set_asset_shares_changed(){
    asset_shares.set_changed();
}

void budget::set_asset_shares_next_id(size_t next_id){
    asset_shares.next_id = next_id;
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

size_t budget::add_asset_share(budget::asset_share& asset_share){
    return asset_shares.add(asset_share);
}

bool budget::edit_asset_share(asset_share& c) {
    return asset_shares.edit(c);
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
