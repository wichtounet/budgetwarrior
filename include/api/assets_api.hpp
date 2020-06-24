//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

namespace httplib {
struct Request;
struct Response;
};

namespace budget {

void add_assets_api(const httplib::Request& req, httplib::Response& res);
void edit_assets_api(const httplib::Request& req, httplib::Response& res);
void delete_assets_api(const httplib::Request& req, httplib::Response& res);
void list_assets_api(const httplib::Request& req, httplib::Response& res);

void add_asset_values_api(const httplib::Request& req, httplib::Response& res);
void edit_asset_values_api(const httplib::Request& req, httplib::Response& res);
void delete_asset_values_api(const httplib::Request& req, httplib::Response& res);
void list_asset_values_api(const httplib::Request& req, httplib::Response& res);

void add_asset_shares_api(const httplib::Request& req, httplib::Response& res);
void edit_asset_shares_api(const httplib::Request& req, httplib::Response& res);
void delete_asset_shares_api(const httplib::Request& req, httplib::Response& res);
void list_asset_shares_api(const httplib::Request& req, httplib::Response& res);

void add_asset_classes_api(const httplib::Request& req, httplib::Response& res);
void edit_asset_classes_api(const httplib::Request& req, httplib::Response& res);
void delete_asset_classes_api(const httplib::Request& req, httplib::Response& res);
void list_asset_classes_api(const httplib::Request& req, httplib::Response& res);

void batch_asset_values_api(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
