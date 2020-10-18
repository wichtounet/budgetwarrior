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

using namespace budget;

namespace {

static data_handler<asset_class> asset_classes { "asset_classes", "asset_classes.data" };

} //end of anonymous namespace

std::map<std::string, std::string> budget::asset_class::get_params() const {
    std::map<std::string, std::string> params;

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

budget::asset_class& budget::get_asset_class(size_t id){
    return asset_classes[id];
}

budget::asset_class& budget::get_asset_class(const std::string & name){
    for (auto& c : asset_classes.data()) {
        if (c.name == name) {
            return c;
        }
    }

    cpp_unreachable("The asset class does not exist");
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

bool budget::asset_class_exists(const std::string& name){
    for (auto& clas : asset_classes.data()) {
        if (clas.name == name) {
            return true;
        }
    }

    return false;
}

std::vector<asset_class>& budget::all_asset_classes(){
    return asset_classes.data();
}

void budget::set_asset_classes_changed(){
    asset_classes.set_changed();
}

void budget::set_asset_class_next_id(size_t next_id){
    asset_classes.next_id = next_id;
}

void budget::show_asset_classes(budget::writer& w){
    if (!asset_classes.size()) {
        w << "No asset classes" << end_of_line;
        return;
    }

    w << title_begin << "Asset Classes" << add_button("asset_classes") << title_end;

    std::vector<std::string> columns = {"ID", "Name", "Edit"};

    std::vector<std::vector<std::string>> contents;

    // Display the asset classes

    for(auto& clas : asset_classes.data()){
        std::vector<std::string> line;

        line.emplace_back(to_string(clas.id));
        line.emplace_back(clas.name);
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

asset_class& budget::asset_class_get(size_t id) {
    if (!asset_classes.exists(id)) {
        throw budget_exception("There are no asset class with id ");
    }

    return asset_classes[id];
}

bool budget::edit_asset_class(asset_class& c) {
    return asset_classes.edit(c);
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
    for (auto & [class_id, class_alloc] : asset.classes) {
        if (class_id == clas.id) {
            return class_alloc;
        }
    }

    return {};
}
