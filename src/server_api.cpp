//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "cpp_utils/assert.hpp"

#include "writer.hpp"
#include "overview.hpp"
#include "expenses.hpp"
#include "accounts.hpp"
#include "assets.hpp"
#include "config.hpp"
#include "objectives.hpp"
#include "wishes.hpp"
#include "version.hpp"
#include "summary.hpp"
#include "fortune.hpp"
#include "recurring.hpp"
#include "guid.hpp"
#include "report.hpp"
#include "debts.hpp"

#include "server_api.hpp"

#include "httplib.h"

using namespace budget;

namespace {

void api_success(const httplib::Request& req, httplib::Response& res, const std::string& message){
    if (req.has_param("server")) {
        auto url = req.params.at("back_page") + "?success=true&message=" + httplib::detail::encode_url(message);
        res.set_redirect(url.c_str());
    } else {
        res.set_content("Success: " + message, "text/plain");
    }
}

void api_success(const httplib::Request& req, httplib::Response& res, const std::string& message, const std::string& content){
    if (req.has_param("server")) {
        auto url = req.params.at("back_page") + "?success=true&message=" + httplib::detail::encode_url(message);
        res.set_redirect(url.c_str());
    } else {
        res.set_content(content, "text/plain");
    }
}

void api_success_content(const httplib::Request& /*req*/, httplib::Response& res, const std::string& content){
    res.set_content(content, "text/plain");
}

void api_error(const httplib::Request& req, httplib::Response& res, const std::string& message){
    if (req.has_param("server")) {
        auto url = req.params.at("back_page") + "?error=true&message=" + httplib::detail::encode_url(message);
        res.set_redirect(url.c_str());
    } else {
        res.set_content("Error: " + message, "text/plain");
    }
}

bool parameters_present(const httplib::Request& req, std::vector<const char*> parameters){
    for(auto& param : parameters){
        if(!req.has_param(param)){
            return false;
        }
    }

    return true;
}

void server_up_api(const httplib::Request& req, httplib::Response& res) {
    api_success_content(req, res, "yes");
}

void add_accounts_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_name", "input_amount"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    account account;
    account.guid   = budget::generate_guid();
    account.name   = req.params.at("input_name");
    account.amount = budget::parse_money(req.params.at("input_amount"));
    account.since  = find_new_since();
    account.until  = budget::date(2099, 12, 31);

    add_account(std::move(account));

    api_success(req, res, "Account " + to_string(account.id) + " has been created", to_string(account.id));
}

void edit_accounts_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id", "input_name", "input_amount"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::account_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "account " + id + " does not exist");
        return;
    }

    account& account = account_get(budget::to_number<size_t>(id));
    account.name     = req.params.at("input_name");
    account.amount   = budget::parse_money(req.params.at("input_amount"));

    set_accounts_changed();

    api_success(req, res, "Account " + to_string(account.id) + " has been modified");
}

void delete_accounts_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::account_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The account " + id + " does not exit");
        return;
    }

    budget::account_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Account " + id + " has been deleted");
}

void list_accounts_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for(auto& account : all_accounts()){
        ss << account;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void archive_accounts_month_api(const httplib::Request& req, httplib::Response& res) {
    budget::archive_accounts_impl(true);

    api_success(req, res, "Accounts have been migrated from the beginning of the month");
}

void archive_accounts_year_api(const httplib::Request& req, httplib::Response& res) {
    budget::archive_accounts_impl(false);

    api_success(req, res, "Accounts have been migrated from the beginning of the year");
}

void add_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_name", "input_date", "input_amount", "input_account"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    expense expense;
    expense.guid    = budget::generate_guid();
    expense.date    = budget::from_string(req.params.at("input_date"));
    expense.account = budget::to_number<size_t>(req.params.at("input_account"));
    expense.name    = req.params.at("input_name");
    expense.amount  = budget::parse_money(req.params.at("input_amount"));

    add_expense(std::move(expense));

    api_success(req, res, "Expense " + to_string(expense.id) + " has been created", to_string(expense.id));
}

void edit_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id", "input_name", "input_date", "input_amount", "input_account"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::expense_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Expense " + id + " does not exist");
        return;
    }

    expense& expense = expense_get(budget::to_number<size_t>(id));
    expense.date     = budget::from_string(req.params.at("input_date"));
    expense.account  = budget::to_number<size_t>(req.params.at("input_account"));
    expense.name     = req.params.at("input_name");
    expense.amount   = budget::parse_money(req.params.at("input_amount"));

    set_expenses_changed();

    api_success(req, res, "Expense " + to_string(expense.id) + " has been modified");
}

void delete_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::expense_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The expense " + id + " does not exit");
        return;
    }

    budget::expense_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Expense " + id + " has been deleted");
}

void list_expenses_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for(auto& expense : all_expenses()){
        ss << expense;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void add_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_name", "input_date", "input_amount", "input_account"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    earning earning;
    earning.guid    = budget::generate_guid();
    earning.date    = budget::from_string(req.params.at("input_date"));
    earning.account = budget::to_number<size_t>(req.params.at("input_account"));
    earning.name    = req.params.at("input_name");
    earning.amount  = budget::parse_money(req.params.at("input_amount"));

    add_earning(std::move(earning));

    api_success(req, res, "Earning " + to_string(earning.id) + " has been created", to_string(earning.id));
}

void edit_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id", "input_name", "input_date", "input_amount", "input_account"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::earning_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Earning " + id + " does not exist");
        return;
    }

    earning& earning = earning_get(budget::to_number<size_t>(id));
    earning.date     = budget::from_string(req.params.at("input_date"));
    earning.account  = budget::to_number<size_t>(req.params.at("input_account"));
    earning.name     = req.params.at("input_name");
    earning.amount   = budget::parse_money(req.params.at("input_amount"));

    set_earnings_changed();

    api_success(req, res, "Earning " + to_string(earning.id) + " has been modified");
}

void delete_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::earning_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The earning " + id + " does not exit");
        return;
    }

    budget::earning_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Earning " + id + " has been deleted");
}

void list_earnings_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for(auto& earning : all_earnings()){
        ss << earning;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void add_objectives_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_name", "input_type", "input_type", "input_source", "input_operator", "input_amount"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    objective objective;
    objective.guid   = budget::generate_guid();
    objective.name   = req.params.at("input_name");
    objective.type   = req.params.at("input_type");
    objective.source = req.params.at("input_source");
    objective.op     = req.params.at("input_operator");
    objective.amount = budget::parse_money(req.params.at("input_amount"));

    add_objective(std::move(objective));

    api_success(req, res, "objective " + to_string(objective.id) + " has been created", to_string(objective.id));
}

void edit_objectives_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id", "input_name", "input_type", "input_type", "input_source", "input_operator", "input_amount"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::objective_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "objective " + id + " does not exist");
        return;
    }

    objective& objective          = objective_get(budget::to_number<size_t>(id));
    objective.name   = req.params.at("input_name");
    objective.type   = req.params.at("input_type");
    objective.source = req.params.at("input_source");
    objective.op     = req.params.at("input_operator");
    objective.amount = budget::parse_money(req.params.at("input_amount"));

    set_objectives_changed();

    api_success(req, res, "objective " + to_string(objective.id) + " has been modified");
}

void delete_objectives_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::objective_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The objective " + id + " does not exit");
        return;
    }

    budget::objective_delete(budget::to_number<size_t>(id));

    api_success(req, res, "objective " + id + " has been deleted");
}

void list_objectives_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for(auto& objective : all_objectives()){
        ss << objective;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void add_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_name", "input_int_stocks", "input_dom_stocks", "input_bonds", "input_cash", "input_portfolio", "input_alloc"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    asset asset;
    asset.guid            = budget::generate_guid();
    asset.name            = req.params.at("input_name");
    asset.int_stocks      = budget::parse_money(req.params.at("input_int_stocks"));
    asset.dom_stocks      = budget::parse_money(req.params.at("input_dom_stocks"));
    asset.bonds           = budget::parse_money(req.params.at("input_bonds"));
    asset.cash            = budget::parse_money(req.params.at("input_cash"));
    asset.portfolio       = req.params.at("input_portfolio") == "yes";
    asset.portfolio_alloc = budget::parse_money(req.params.at("input_alloc"));
    asset.currency        = req.params.at("input_currency");

    if(asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != money(100)){
        api_error(req, res, "The total allocation of the asset is not 100%");
        return;
    }

    add_asset(std::move(asset));

    api_success(req, res, "asset " + to_string(asset.id) + " has been created", to_string(asset.id));
}

void edit_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id", "input_name", "input_int_stocks", "input_dom_stocks", "input_bonds", "input_cash", "input_portfolio", "input_alloc"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::asset_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "asset " + id + " does not exist");
        return;
    }

    asset& asset          = asset_get(budget::to_number<size_t>(id));
    asset.name            = req.params.at("input_name");
    asset.int_stocks      = budget::parse_money(req.params.at("input_int_stocks"));
    asset.dom_stocks      = budget::parse_money(req.params.at("input_dom_stocks"));
    asset.bonds           = budget::parse_money(req.params.at("input_bonds"));
    asset.cash            = budget::parse_money(req.params.at("input_cash"));
    asset.portfolio       = req.params.at("input_portfolio") == "yes";
    asset.portfolio_alloc = budget::parse_money(req.params.at("input_alloc"));
    asset.currency        = req.params.at("input_currency");

    if(asset.int_stocks + asset.dom_stocks + asset.bonds + asset.cash != money(100)){
        api_error(req, res, "The total allocation of the asset is not 100%");
        return;
    }

    set_assets_changed();

    api_success(req, res, "asset " + to_string(asset.id) + " has been modified");
}

void delete_assets_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::asset_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The asset " + id + " does not exit");
        return;
    }

    budget::asset_delete(budget::to_number<size_t>(id));

    api_success(req, res, "asset " + id + " has been deleted");
}

void list_assets_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for(auto& asset : all_assets()){
        ss << asset;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void add_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_asset", "input_date", "input_amount"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    asset_value asset_value;
    asset_value.guid     = budget::generate_guid();
    asset_value.amount   = budget::parse_money(req.params.at("input_amount"));
    asset_value.asset_id = budget::to_number<size_t>(req.params.at("input_asset"));
    asset_value.set_date = budget::from_string(req.params.at("input_date"));

    add_asset_value(std::move(asset_value));

    api_success(req, res, "Asset value " + to_string(asset_value.id) + " has been created", to_string(asset_value.id));
}

void edit_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id", "input_asset", "input_date", "input_amount"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::asset_value_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Asset value " + id + " does not exist");
        return;
    }

    asset_value& asset_value = asset_value_get(budget::to_number<size_t>(id));
    asset_value.amount       = budget::parse_money(req.params.at("input_amount"));
    asset_value.asset_id     = budget::to_number<size_t>(req.params.at("input_asset"));
    asset_value.set_date     = budget::from_string(req.params.at("input_date"));

    set_asset_values_changed();

    api_success(req, res, "Asset " + to_string(asset_value.id) + " has been modified");
}

void delete_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id"})){
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::asset_value_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The asset value " + id + " does not exit");
        return;
    }

    budget::asset_value_delete(budget::to_number<size_t>(id));

    api_success(req, res, "The asset value " + id + " has been deleted");
}

void list_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for(auto& asset_value : all_asset_values()){
        ss << asset_value;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void batch_asset_values_api(const httplib::Request& req, httplib::Response& res) {
    auto sorted_asset_values = all_asset_values();

    std::sort(sorted_asset_values.begin(), sorted_asset_values.end(),
              [](const budget::asset_value& a, const budget::asset_value& b) { return a.set_date < b.set_date; });

    for(auto& asset : all_assets()){
        auto input_name = "input_amount_" + budget::to_string(asset.id);

        if (req.has_param(input_name.c_str())) {
            auto new_amount = budget::parse_money(req.params.at(input_name.c_str()));

            budget::money current_amount;

            for(auto& asset_value : sorted_asset_values){
                if(asset_value.asset_id == asset.id){
                    current_amount = asset_value.amount;
                }
            }

            // If the amount changed, update it
            if(current_amount != new_amount){
                asset_value asset_value;
                asset_value.guid         = budget::generate_guid();
                asset_value.amount       = new_amount;
                asset_value.asset_id     = asset.id;
                asset_value.set_date     = budget::from_string(req.params.at("input_date"));

                add_asset_value(std::move(asset_value));
            }
        }
    }

    api_success(req, res, "Asset values have been updated");
}

void add_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_account")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    recurring recurring;
    recurring.guid    = budget::generate_guid();
    recurring.account = budget::get_account(budget::to_number<size_t>(req.params.at("input_account"))).name;
    recurring.name    = req.params.at("input_name");
    recurring.amount  = budget::parse_money(req.params.at("input_amount"));
    recurring.recurs  = "monthly";

    add_recurring(std::move(recurring));

    api_success(req, res, "Recurring " + to_string(recurring.id) + " has been created", to_string(recurring.id));
}

void edit_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_account")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::recurring_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "recurring " + id + " does not exist");
        return;
    }

    recurring& recurring = recurring_get(budget::to_number<size_t>(id));
    recurring.account  = budget::get_account(budget::to_number<size_t>(req.params.at("input_account"))).name;
    recurring.name     = req.params.at("input_name");
    recurring.amount   = budget::parse_money(req.params.at("input_amount"));

    set_recurrings_changed();

    api_success(req, res, "Recurring " + to_string(recurring.id) + " has been modified");
}

void delete_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::recurring_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The recurring " + id + " does not exit");
        return;
    }

    budget::recurring_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Recurring " + id + " has been deleted");
}

void list_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for(auto& recurring : all_recurrings()){
        ss << recurring;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void add_debts_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_title") || !req.has_param("input_direction")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    debt debt;
    debt.state         = 0;
    debt.guid          = budget::generate_guid();
    debt.creation_date = budget::local_day();
    debt.direction     = req.params.at("input_direction") == "to";
    debt.name          = req.params.at("input_name");
    debt.title         = req.params.at("input_title");
    debt.amount        = budget::parse_money(req.params.at("input_amount"));

    add_debt(std::move(debt));

    api_success(req, res, "Debt " + to_string(debt.id) + " has been created", to_string(debt.id));
}

void edit_debts_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_title") || !req.has_param("input_direction") || !req.has_param("input_paid")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::debt_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Debt " + id + " does not exist");
        return;
    }

    debt& debt = debt_get(budget::to_number<size_t>(id));
    debt.direction = req.params.at("input_direction") == "to";
    debt.name      = req.params.at("input_name");
    debt.title     = req.params.at("input_title");
    debt.amount    = budget::parse_money(req.params.at("input_amount"));
    debt.state     = req.params.at("input_paid") == "yes" ? 1 : 0;

    set_debts_changed();

    api_success(req, res, "Debt " + to_string(debt.id) + " has been modified");
}

void delete_debts_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::debt_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The debt " + id + " does not exit");
        return;
    }

    budget::debt_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Debt " + id + " has been deleted");
}

void list_debts_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for(auto& debt : all_debts()){
        ss << debt;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void add_fortunes_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_amount") || !req.has_param("input_date")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    fortune fortune;
    fortune.guid       = budget::generate_guid();
    fortune.check_date = budget::from_string(req.params.at("input_date"));
    fortune.amount     = budget::parse_money(req.params.at("input_amount"));

    add_fortune(std::move(fortune));

    api_success(req, res, "Fortune " + to_string(fortune.id) + " has been created", to_string(fortune.id));
}

void edit_fortunes_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id") || !req.has_param("input_amount") || !req.has_param("input_date")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::fortune_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "Fortune " + id + " does not exist");
        return;
    }

    fortune& fortune   = fortune_get(budget::to_number<size_t>(id));
    fortune.check_date = budget::from_string(req.params.at("input_date"));
    fortune.amount     = budget::parse_money(req.params.at("input_amount"));

    set_fortunes_changed();

    api_success(req, res, "Fortune " + to_string(fortune.id) + " has been modified");
}

void delete_fortunes_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::fortune_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The fortune " + id + " does not exit");
        return;
    }

    budget::fortune_delete(budget::to_number<size_t>(id));

    api_success(req, res, "fortune " + id + " has been deleted");
}

void list_fortunes_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for(auto& fortune : all_fortunes()){
        ss << fortune;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

void add_wishes_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_urgency") || !req.has_param("input_importance")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    wish wish;
    wish.paid        = false;
    wish.paid_amount = 0;
    wish.guid        = budget::generate_guid();
    wish.date        = budget::local_day();
    wish.name        = req.params.at("input_name");
    wish.importance  = budget::to_number<int>(req.params.at("input_importance"));
    wish.urgency     = budget::to_number<int>(req.params.at("input_urgency"));
    wish.amount      = budget::parse_money(req.params.at("input_amount"));

    add_wish(std::move(wish));

    api_success(req, res, "wish " + to_string(wish.id) + " has been created", to_string(wish.id));
}

void edit_wishes_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_urgency")
                || !req.has_param("input_importance") || !req.has_param("input_paid") || !req.has_param("input_paid_amount")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::wish_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "wish " + id + " does not exist");
        return;
    }

    bool paid = req.params.at("input_paid") == "yes";

    wish& wish = wish_get(budget::to_number<size_t>(id));
    wish.name       = req.params.at("input_name");
    wish.importance = budget::to_number<int>(req.params.at("input_importance"));
    wish.urgency    = budget::to_number<int>(req.params.at("input_urgency"));
    wish.amount     = budget::parse_money(req.params.at("input_amount"));
    wish.paid       = paid;

    if (paid) {
        wish.paid_amount = budget::parse_money(req.params.at("input_paid_amount"));
    }

    set_wishes_changed();

    api_success(req, res, "wish " + to_string(wish.id) + " has been modified");
}

void delete_wishes_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id")) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    auto id = req.params.at("input_id");

    if (!budget::wish_exists(budget::to_number<size_t>(id))) {
        api_error(req, res, "The wish " + id + " does not exit");
        return;
    }

    budget::wish_delete(budget::to_number<size_t>(id));

    api_success(req, res, "wish " + id + " has been deleted");
}

void list_wishes_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for(auto& wish : all_wishes()){
        ss << wish;
        ss << std::endl;
    }

    api_success_content(req, res, ss.str());
}

} //end of anonymous namespace

void budget::load_api(httplib::Server& server){
    server.get("/api/server/up/", &server_up_api);

    server.post("/api/accounts/add/", &add_accounts_api);
    server.post("/api/accounts/edit/", &edit_accounts_api);
    server.post("/api/accounts/delete/", &delete_accounts_api);
    server.post("/api/accounts/archive/month/", &archive_accounts_month_api);
    server.post("/api/accounts/archive/year/", &archive_accounts_year_api);
    server.get("/api/accounts/list/", &list_accounts_api);

    server.post("/api/expenses/add/", &add_expenses_api);
    server.post("/api/expenses/edit/", &edit_expenses_api);
    server.post("/api/expenses/delete/", &delete_expenses_api);
    server.get("/api/expenses/list/", &list_expenses_api);

    server.post("/api/earnings/add/", &add_earnings_api);
    server.post("/api/earnings/edit/", &edit_earnings_api);
    server.post("/api/earnings/delete/", &delete_earnings_api);
    server.get("/api/earnings/list/", &list_earnings_api);

    server.post("/api/recurrings/add/", &add_recurrings_api);
    server.post("/api/recurrings/edit/", &edit_recurrings_api);
    server.post("/api/recurrings/delete/", &delete_recurrings_api);
    server.get("/api/recurrings/list/", &list_recurrings_api);

    server.post("/api/debts/add/", &add_debts_api);
    server.post("/api/debts/edit/", &edit_debts_api);
    server.post("/api/debts/delete/", &delete_debts_api);
    server.get("/api/debts/list/", &list_debts_api);

    server.post("/api/fortunes/add/", &add_fortunes_api);
    server.post("/api/fortunes/edit/", &edit_fortunes_api);
    server.post("/api/fortunes/delete/", &delete_fortunes_api);
    server.get("/api/fortunes/list/", &list_fortunes_api);

    server.post("/api/wishes/add/", &add_wishes_api);
    server.post("/api/wishes/edit/", &edit_wishes_api);
    server.post("/api/wishes/delete/", &delete_wishes_api);
    server.get("/api/wishes/list/", &list_wishes_api);

    server.post("/api/assets/add/", &add_assets_api);
    server.post("/api/assets/edit/", &edit_assets_api);
    server.post("/api/assets/delete/", &delete_assets_api);
    server.get("/api/assets/list/", &list_assets_api);

    server.post("/api/asset_values/add/", &add_asset_values_api);
    server.post("/api/asset_values/edit/", &edit_asset_values_api);
    server.post("/api/asset_values/batch/", &batch_asset_values_api);
    server.post("/api/asset_values/delete/", &delete_asset_values_api);
    server.get("/api/asset_values/list/", &list_asset_values_api);

    server.post("/api/objectives/add/", &add_objectives_api);
    server.post("/api/objectives/edit/", &edit_objectives_api);
    server.post("/api/objectives/delete/", &delete_objectives_api);
    server.get("/api/objectives/list/", &list_objectives_api);
}
