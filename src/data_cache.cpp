//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "data_cache.hpp"
#include "views.hpp"

using namespace budget;

std::vector<earning> & data_cache::earnings() {
    if (earnings_.empty()) {
        earnings_ = all_earnings();
    }

    return earnings_;
}

std::vector<earning> & data_cache::sorted_earnings() {
    if (sorted_earnings_.empty()) {
        sorted_earnings_ = all_earnings();

        std::ranges::sort(sorted_earnings_, [](auto& lhs, auto& rhs) {
            return lhs.date < rhs.date;
        });
    }

    return sorted_earnings_;
}

std::vector<debt> & data_cache::debts() {
    if (debts_.empty()) {
        debts_ = all_debts();
    }

    return debts_;
}

std::vector<fortune> & data_cache::fortunes() {
    if (fortunes_.empty()) {
        fortunes_ = all_fortunes();
    }

    return fortunes_;
}

std::vector<asset_value> & data_cache::asset_values() {
    if (asset_values_.empty()) {
        asset_values_ = all_asset_values();
    }

    return asset_values_;
}

std::vector<asset_value> & data_cache::sorted_asset_values() {
    if (sorted_asset_values_.empty()) {
        sorted_asset_values_ = all_asset_values();

        std::ranges::stable_sort(sorted_asset_values_, [](auto& lhs, auto& rhs) {
            return lhs.set_date < rhs.set_date;
        });
    }

    return sorted_asset_values_;
}

std::unordered_map<size_t, std::vector<asset_value>> & data_cache::sorted_group_asset_values(bool liability) {
    if (liability) {
        if (sorted_group_asset_values_liabilities_.empty()) {
            for (auto& asset_value : sorted_asset_values() | liability_only) {
                sorted_group_asset_values_liabilities_[asset_value.asset_id].push_back(asset_value);
            }
        }

        return sorted_group_asset_values_liabilities_;
    }
    if (sorted_group_asset_values_.empty()) {
        for (auto& asset_value : sorted_asset_values() | not_liability) {
            sorted_group_asset_values_[asset_value.asset_id].push_back(asset_value);
        }
    }

    return sorted_group_asset_values_;
}

std::vector<liability> & data_cache::liabilities() {
    if (liabilities_.empty()) {
        liabilities_ = all_liabilities();
    }

    return liabilities_;
}

std::vector<recurring> & data_cache::recurrings() {
    if (recurrings_.empty()) {
        recurrings_ = all_recurrings();
    }

    return recurrings_;
}

std::vector<income> & data_cache::incomes() {
    if (incomes_.empty()) {
        incomes_ = all_incomes();
    }

    return incomes_;
}

std::vector<account> & data_cache::accounts() {
    if (accounts_.empty()) {
        accounts_ = all_accounts();
    }

    return accounts_;
}

std::vector<asset_share> & data_cache::asset_shares() {
    if (asset_shares_.empty()) {
        asset_shares_ = all_asset_shares();
    }

    return asset_shares_;
}

std::vector<asset_share> & data_cache::sorted_asset_shares() {
    if (sorted_asset_shares_.empty()) {
        sorted_asset_shares_ = asset_shares();

        std::ranges::sort(sorted_asset_shares_, [](auto& lhs, auto& rhs) {
            return lhs.date < rhs.date;
        });
    }

    return sorted_asset_shares_;
}

std::vector<asset_class> & data_cache::asset_classes() {
    if (asset_classes_.empty()) {
        asset_classes_ = all_asset_classes();
    }

    return asset_classes_;
}

std::vector<objective> & data_cache::objectives() {
    if (objectives_.empty()) {
        objectives_ = all_objectives();
    }

    return objectives_;
}

std::vector<expense> & data_cache::expenses() {
    if (expenses_.empty()) {
        expenses_ = all_expenses();
    }

    return expenses_;
}

std::vector<expense> & data_cache::sorted_expenses() {
    if (sorted_expenses_.empty()) {
        sorted_expenses_ = expenses();

        std::ranges::sort(sorted_expenses_, [](auto& lhs, auto& rhs) {
            return lhs.date < rhs.date;
        });
    }

    return sorted_expenses_;
}

std::vector<asset> & data_cache::assets() {
    if (assets_.empty()) {
        assets_ = all_assets();
    }

    return assets_;
}

const std::vector<asset> & data_cache::user_assets() {
    if (user_assets_.empty()) {
        std::ranges::copy(assets() | is_user, std::back_inserter(user_assets_));
    }

    return user_assets_;
}

std::vector<asset> & data_cache::active_user_assets() {
    if (active_user_assets_.empty()) {
        std::ranges::copy(assets() | is_user | is_active, std::back_inserter(user_assets_));
    }

    return active_user_assets_;
}

std::vector<wish> & data_cache::wishes() {
    if (wishes_.empty()) {
        wishes_ = all_wishes();
    }

    return wishes_;
}

