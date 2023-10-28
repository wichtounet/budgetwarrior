//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "assets.hpp"
#include "budget_exception.hpp"
#include "data.hpp"
#include "guid.hpp"
#include "writer.hpp"
#include "views.hpp"

using namespace budget;

namespace {

data_handler<asset_class> asset_classes{"asset_classes", "asset_classes.data"};

} //end of anonymous namespace

std::map<std::string, std::string, std::less<>> budget::asset_class::get_params() const {
    std::map<std::string, std::string, std::less<>> params;

    params["input_id"]   = budget::to_string(id);
    params["input_guid"] = guid;
    params["input_name"] = name;

    return params;
}

void budget::load_asset_classes() {
    asset_classes.load();
}

void budget::save_asset_classes() {
    asset_classes.save();
}

budget::asset_class budget::get_asset_class(size_t id){
    return asset_classes[id];
}

budget::asset_class budget::get_asset_class(std::string_view name){
    if (auto range = asset_classes.data() | filter_by_name(name); range) {
        return *std::ranges::begin(range);
    }

    cpp_unreachable("The asset class does not exist");
}

void budget::asset_class::save(data_writer& writer) const {
    writer << id;
    writer << guid;
    writer << name;
    writer << fi;
}

void budget::asset_class::load(data_reader & reader){
    reader >> id;
    reader >> guid;
    reader >> name;

    if (reader.peek().empty() || reader.peek() == " ") {
        fi = false;
    } else {
        reader >> fi;
    }
}

bool budget::asset_class_exists(const std::string& name){
    return !!(asset_classes.data() | filter_by_name(name));
}

std::vector<asset_class> budget::all_asset_classes(){
    return asset_classes.data();
}

void budget::set_asset_classes_changed(){
    asset_classes.set_changed();
}

void budget::set_asset_class_next_id(size_t next_id){
    asset_classes.next_id = next_id;
}

void budget::show_asset_classes(budget::writer& w){
    if (asset_classes.empty()) {
        w << "No asset classes" << end_of_line;
        return;
    }

    w << title_begin << "Asset Classes" << add_button("asset_classes") << title_end;

    std::vector<std::string> columns = {"ID", "Name", "FI", "Edit"};

    std::vector<std::vector<std::string>> contents;

    // Display the asset classes

    for(auto& clas : asset_classes.data()){
        std::vector<std::string> line;

        line.emplace_back(to_string(clas.id));
        line.emplace_back(clas.name);
        line.emplace_back(clas.fi ? "yes" : "no");
        line.emplace_back("::edit::asset_classes::" + budget::to_string(clas.id));

        contents.emplace_back(std::move(line));
    }

    w.display_table(columns, contents);
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

bool budget::edit_asset_class(asset_class& c) {
    return asset_classes.indirect_edit(c);
}

size_t budget::add_asset_class(budget::asset_class& asset){
    return asset_classes.add(asset);
}

void budget::update_asset_class_allocation(budget::asset& asset, budget::asset_class & clas, budget::money alloc) {
    for (auto & [class_id, class_alloc] : asset.classes) {
        if (class_id == clas.id) {
            class_alloc = alloc;
            return;
        }
    }

    asset.classes.emplace_back(clas.id, alloc);
}

budget::money budget::get_asset_class_allocation(const budget::asset& asset, const budget::asset_class & clas) {
    for (const auto& [class_id, class_alloc] : asset.classes) {
        if (class_id == clas.id) {
            return class_alloc;
        }
    }

    return {};
}

void budget::migrate_assets_8_to_9(){
    asset_classes.load([](data_reader & reader, asset_class& asset_class){
        reader >> asset_class.id;
        reader >> asset_class.guid;
        reader >> asset_class.name;

        // Version 9 added support for FI Net Worth
        asset_class.fi = true;
    });

    set_asset_classes_changed();

    asset_classes.save();
}
