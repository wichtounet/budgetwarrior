//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/assets_api.hpp"

#include "assets.hpp"
#include "liabilities.hpp"
#include "accounts.hpp"
#include "guid.hpp"
#include "http.hpp"
#include "data.hpp"
#include "budget_exception.hpp"

using namespace budget;

void budget::add_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_name", "input_portfolio", "input_alloc", "input_share_based", "input_ticker"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    try {
        asset asset;
        asset.guid = budget::generate_guid();
        asset.name = req.get_param_value("input_name");

        for (auto& clas : all_asset_classes()) {
            auto param_name = "input_class_" + to_string(clas.id);

            if (req.has_param(param_name.c_str())) {
                update_asset_class_allocation(asset, clas, budget::money_from_string(req.get_param_value(param_name.c_str())));
            }
        }

        // Portfolio allocation
        asset.portfolio = req.get_param_value("input_portfolio") == "yes";
        if (asset.portfolio) {
            asset.portfolio_alloc = budget::money_from_string(req.get_param_value("input_alloc"));
        }

        asset.currency = req.get_param_value("input_currency");

        // Share
        asset.share_based = req.get_param_value("input_share_based") == "yes";
        if (asset.share_based) {
            asset.ticker = req.get_param_value("input_ticker");

            if (asset.ticker.empty()) {
                api_error(req, res, "The ticker cannot be empty for a shared-based asset");
                return;
            }
        }

        if (asset.total_allocation() != money(100)) {
            api_error(req, res, "The total allocation of the asset is not 100%");
            return;
        }

        add_asset(asset);

        api_success(req, res, "asset " + to_string(asset.id) + " has been created", to_string(asset.id));
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::edit_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_name", "input_portfolio", "input_alloc", "input_share_based", "input_ticker"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "asset " + id + " does not exist");
        return;
    }

    try {
        asset asset = get_asset(budget::to_number<size_t>(id));
        asset.name  = req.get_param_value("input_name");

        for (auto& clas : all_asset_classes()) {
            auto param_name = "input_class_" + to_string(clas.id);

            if (req.has_param(param_name.c_str())) {
                update_asset_class_allocation(asset, clas, budget::money_from_string(req.get_param_value(param_name.c_str())));
            }
        }

        asset.portfolio       = req.get_param_value("input_portfolio") == "yes";
        asset.portfolio_alloc = budget::money_from_string(req.get_param_value("input_alloc"));
        asset.currency        = req.get_param_value("input_currency");
        asset.share_based     = req.get_param_value("input_share_based") == "yes";
        asset.ticker          = req.get_param_value("input_ticker");

        if (asset.total_allocation() != money(100)) {
            api_error(req, res, "The total allocation of the asset is not 100%");
            return;
        }

        edit_asset(asset);

        api_success(req, res, "asset " + to_string(asset.id) + " has been modified");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::delete_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The asset " + id + " does not exit");
        return;
    }

    try {
        budget::asset_delete(budget::to_number<size_t>(id));

        api_success(req, res, "asset " + id + " has been deleted");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::list_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        std::stringstream ss;

        for (auto& asset : all_assets()) {
            data_writer writer;
            asset.save(writer);
            ss << writer.to_string() << std::endl;
        }

        api_success_content(req, res, ss.str());
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::add_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_asset", "input_date", "input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    try {
        asset_value asset_value;
        asset_value.guid      = budget::generate_guid();
        asset_value.amount    = budget::money_from_string(req.get_param_value("input_amount"));
        asset_value.asset_id  = budget::to_number<size_t>(req.get_param_value("input_asset"));
        asset_value.set_date  = budget::date_from_string(req.get_param_value("input_date"));
        asset_value.liability = req.get_param_value("input_liability") == "true";

        add_asset_value(asset_value);

        api_success(req, res, "Asset value " + to_string(asset_value.id) + " has been created", to_string(asset_value.id));
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::edit_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_asset", "input_date", "input_amount"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_value_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Asset value " + id + " does not exist");
        return;
    }

    try {
        asset_value asset_value = get_asset_value(budget::to_number<size_t>(id));
        asset_value.amount      = budget::money_from_string(req.get_param_value("input_amount"));
        asset_value.asset_id    = budget::to_number<size_t>(req.get_param_value("input_asset"));
        asset_value.set_date    = budget::date_from_string(req.get_param_value("input_date"));
        asset_value.liability   = req.get_param_value("input_liability") == "true";

        edit_asset_value(asset_value);

        api_success(req, res, "Asset " + to_string(asset_value.id) + " has been modified");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::delete_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_value_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The asset value " + id + " does not exit");
        return;
    }

    try {
        budget::asset_value_delete(budget::to_number<size_t>(id));

        api_success(req, res, "The asset value " + id + " has been deleted");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::list_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        std::stringstream ss;

        for (auto& asset_value : all_asset_values()) {
            data_writer writer;
            asset_value.save(writer);
            ss << writer.to_string() << std::endl;
        }

        api_success_content(req, res, ss.str());
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::batch_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        auto asset_values = all_asset_values();

        for (auto& asset : all_assets()) {
            auto input_name = "input_amount_" + budget::to_string(asset.id);

            if (req.has_param(input_name.c_str())) {
                auto new_amount = budget::money_from_string(req.get_param_value(input_name.c_str()));

                budget::money current_amount;

                for (auto& asset_value : asset_values) {
                    if (asset_value.asset_id == asset.id) {
                        current_amount = asset_value.amount;
                    }
                }

                // If the amount changed, update it
                if (current_amount != new_amount) {
                    asset_value asset_value;
                    asset_value.guid      = budget::generate_guid();
                    asset_value.amount    = new_amount;
                    asset_value.asset_id  = asset.id;
                    asset_value.set_date  = budget::date_from_string(req.get_param_value("input_date"));
                    asset_value.liability = false;

                    add_asset_value(asset_value);
                }
            }
        }

        api_success(req, res, "Asset values have been updated");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::add_asset_shares_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_asset", "input_shares", "input_price", "input_date"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    try {
        asset_share asset_share;
        asset_share.guid     = budget::generate_guid();
        asset_share.asset_id = budget::to_number<size_t>(req.get_param_value("input_asset"));
        asset_share.shares   = budget::to_number<int64_t>(req.get_param_value("input_shares"));
        asset_share.price    = budget::money_from_string(req.get_param_value("input_price"));
        asset_share.date     = budget::date_from_string(req.get_param_value("input_date"));

        add_asset_share(asset_share);

        api_success(req, res, "Asset share " + to_string(asset_share.id) + " has been created", to_string(asset_share.id));
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::edit_asset_shares_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_asset", "input_shares", "input_price", "input_date"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_share_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Asset share " + id + " does not exist");
        return;
    }

    try {
        asset_share asset_share = get_asset_share(budget::to_number<size_t>(id));
        asset_share.asset_id    = budget::to_number<size_t>(req.get_param_value("input_asset"));
        asset_share.shares      = budget::to_number<int64_t>(req.get_param_value("input_shares"));
        asset_share.price       = budget::money_from_string(req.get_param_value("input_price"));
        asset_share.date        = budget::date_from_string(req.get_param_value("input_date"));

        edit_asset_share(asset_share);

        api_success(req, res, "Asset " + to_string(asset_share.id) + " has been modified");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::delete_asset_shares_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_share_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The asset share " + id + " does not exit");
        return;
    }

    try {
        budget::asset_share_delete(budget::to_number<size_t>(id));

        api_success(req, res, "The asset share " + id + " has been deleted");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::list_asset_shares_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        std::stringstream ss;

        for (auto& asset_share : all_asset_shares()) {
            data_writer writer;
            asset_share.save(writer);
            ss << writer.to_string() << std::endl;
        }

        api_success_content(req, res, ss.str());
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

// Asset Classes

void budget::add_asset_classes_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_name"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    try {
        asset_class asset_class;
        asset_class.guid = budget::generate_guid();
        asset_class.name = req.get_param_value("input_name");

        add_asset_class(asset_class);

        api_success(req, res, "Asset class " + to_string(asset_class.id) + " has been created", to_string(asset_class.id));
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::edit_asset_classes_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_name"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_class_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Asset class " + id + " does not exist");
        return;
    }

    try {
        asset_class asset_class = get_asset_class(budget::to_number<size_t>(id));
        asset_class.name        = req.get_param_value("input_name");

        edit_asset_class(asset_class);

        api_success(req, res, "Asset Class " + to_string(asset_class.id) + " has been modified");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::delete_asset_classes_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::asset_class_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The asset class " + id + " does not exit");
        return;
    }

    try {
        auto clas = get_asset_class(budget::to_number<size_t>(id));

        for (auto& asset : all_assets()) {
            if (get_asset_class_allocation(asset, clas)) {
                api_error(req, res, "Cannot delete an asset class that is still used");
                return;
            }
        }

        budget::asset_class_delete(budget::to_number<size_t>(id));

        api_success(req, res, "The asset class " + id + " has been deleted");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::list_asset_classes_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        std::stringstream ss;

        for (auto& asset_class : all_asset_classes()) {
            data_writer writer;
            asset_class.save(writer);
            ss << writer.to_string() << std::endl;
        }

        api_success_content(req, res, ss.str());
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

// Liabilities

void budget::add_liabilities_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_name", "input_currency"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    try {
        liability liability;
        liability.guid     = budget::generate_guid();
        liability.name     = req.get_param_value("input_name");
        liability.currency = req.get_param_value("input_currency");

        add_liability(liability);

        api_success(req, res, "Liability " + to_string(liability.id) + " has been created", to_string(liability.id));
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::edit_liabilities_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id", "input_name", "input_currency"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::liability_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Liability " + id + " does not exist");
        return;
    }

    try {
        liability liability = get_liability(budget::to_number<size_t>(id));
        liability.name      = req.get_param_value("input_name");
        liability.currency  = req.get_param_value("input_currency");

        edit_liability(liability);

        api_success(req, res, "Liability " + to_string(liability.id) + " has been modified");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::delete_liabilities_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    if (!parameters_present(req, {"input_id"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.get_param_value("input_id");

    if (!budget::liability_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The Liability " + id + " does not exit");
        return;
    }

    try {
        budget::liability_delete(budget::to_number<size_t>(id));

        api_success(req, res, "The liability " + id + " has been deleted");
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}

void budget::list_liabilities_api(const httplib::Request& req, httplib::Response& res) {
    if (!api_start(req, res)) {
        return;
    }

    try {
        std::stringstream ss;

        for (auto& liability : all_liabilities()) {
            data_writer writer;
            liability.save(writer);
            ss << writer.to_string() << std::endl;
        }

        api_success_content(req, res, ss.str());
    } catch (const budget_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    } catch (const date_exception& e) {
        api_error(req, res, "Exception occurred: " + e.message());
    }
}
