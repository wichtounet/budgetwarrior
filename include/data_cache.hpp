//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <vector>

#include "earnings.hpp"
#include "debts.hpp"
#include "fortune.hpp"
#include "assets.hpp"
#include "liabilities.hpp"
#include "recurring.hpp"
#include "incomes.hpp"
#include "accounts.hpp"
#include "objectives.hpp"
#include "expenses.hpp"
#include "wishes.hpp"
#include <unordered_map>

namespace budget {

struct data_cache {
    std::vector<earning> & earnings();
    std::vector<earning> & sorted_earnings();
    std::vector<debt> & debts();
    std::vector<fortune> & fortunes();
    std::vector<asset_value> & asset_values();
    std::vector<asset_value> & sorted_asset_values();
    std::unordered_map<size_t, std::vector<asset_value>> & sorted_group_asset_values(bool liability);
    std::vector<liability> & liabilities();
    std::vector<recurring> & recurrings();
    std::vector<income> & incomes();
    std::vector<account> & accounts();
    std::vector<asset_share> & asset_shares();
    std::vector<asset_share> & sorted_asset_shares();
    std::vector<asset_class> & asset_classes();
    std::vector<objective> & objectives();
    std::vector<expense> & expenses();
    std::vector<expense> & sorted_expenses();
    std::vector<asset> & assets();
    std::vector<asset> & user_assets();
    std::vector<asset> & active_user_assets();
    std::vector<wish> & wishes();

    data_cache() = default;

    // No point in copying that
    data_cache(const data_cache & cache) = delete;
    data_cache & operator=(const data_cache & cache) = delete;

private:
    std::vector<earning> earnings_;
    std::vector<earning> sorted_earnings_;
    std::vector<debt> debts_;
    std::vector<fortune> fortunes_;
    std::vector<asset_value> asset_values_;
    std::vector<asset_value> sorted_asset_values_;
    std::unordered_map<size_t, std::vector<asset_value>> sorted_group_asset_values_;
    std::unordered_map<size_t, std::vector<asset_value>> sorted_group_asset_values_liabilities_;
    std::vector<liability> liabilities_;
    std::vector<recurring> recurrings_;
    std::vector<income> incomes_;
    std::vector<account> accounts_;
    std::vector<asset_share> asset_shares_;
    std::vector<asset_share> sorted_asset_shares_;
    std::vector<asset_class> asset_classes_;
    std::vector<objective> objectives_;
    std::vector<expense> expenses_;
    std::vector<expense> sorted_expenses_;
    std::vector<asset> assets_;
    std::vector<asset> user_assets_;
    std::vector<asset> active_user_assets_;
    std::vector<wish> wishes_;
};

} //end of namespace budget
