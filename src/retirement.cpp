//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "retirement.hpp"
#include "assets.hpp"
#include "accounts.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "config.hpp"
#include "console.hpp"

using namespace budget;

namespace {

constexpr size_t running_limit = 12;

money running_expenses(){
    auto today = budget::local_day();

    budget::date end = today - budget::days(today.day() - 1);
    budget::date start = end - budget::months(running_limit);

    budget::money total;

    for(auto& expense : all_expenses()){
        if(expense.date >= start && expense.date < end){
            total += expense.amount;
        }
    }

    return total;
}

double running_savings_rate(){
    auto today = budget::local_day();

    double savings_rate = 0.0;

    for(size_t i = 1; i <= running_limit; ++i){
        auto d = today - budget::months(i);

        budget::money expenses;

        for (auto& expense : all_expenses()) {
            if (expense.date.year() == d.year() && expense.date.month() == d.month()) {
                expenses += expense.amount;
            }
        }

        budget::money earnings;

        for (auto& earning : all_earnings()) {
            if (earning.date.year() == d.year() && earning.date.month() == d.month()) {
                earnings += earning.amount;
            }
        }

        budget::money income;

        for (auto& account : all_accounts(d.year(), d.month())){
            income += account.amount;
        }

        auto balance = income + earnings - expenses;
        auto local   = balance / (income + earnings);

        if(local < 0){
            local = 0;
        }

        savings_rate += local;
    }

    return savings_rate / running_limit;
}

void retirement_status() {
    if(!internal_config_contains("withdrawal_rate")){
        std::cerr << "Not enough information, please configure first with retirement set" << std::endl;
        return;
    }

    if(!internal_config_contains("expected_roi")){
        std::cerr << "Not enough information, please configure first with retirement set" << std::endl;
        return;
    }

    auto currency = get_default_currency();
    auto wrate = to_number<double>(internal_config_value("withdrawal_rate"));
    auto roi = to_number<double>(internal_config_value("expected_roi"));
    auto years = double(int(100.0 / wrate));
    auto expenses = running_expenses();
    auto savings_rate = running_savings_rate();
    auto nw = get_net_worth();
    auto missing = years * expenses - nw;
    auto income= 12 * get_base_income();

    auto current_nw = nw;
    size_t base_months = 0;

    while(current_nw < years * expenses){
        current_nw *= 1.0 + (roi / 100.0) / 12;
        current_nw += (savings_rate * income) / 12;

        ++base_months;
    }

    std::cout << "         Withdrawal rate: " << wrate << "%" << std::endl;
    std::cout << "        Years of expense: " << years << std::endl;
    std::cout << "        Running expenses: " << expenses << " " << currency << std::endl;
    std::cout << "        Target Net Worth: " << years * expenses << " " << currency << std::endl;
    std::cout << "       Current Net Worth: " << nw << " " << currency << std::endl;
    std::cout << "       Missing Net Worth: " << missing << " " << currency << std::endl;
    std::cout << "                FI Ratio: " << 100 * (nw / missing) << "%" << std::endl;
    std::cout << "           Yearly income: " << income << " " << currency << std::endl;
    std::cout << "    Running Savings Rate: " << 100 * savings_rate << "%" << std::endl;
    std::cout << "          Yearly savings: " << savings_rate * income << " " << currency << std::endl;
    std::cout << "Time to FI (w/o returns): " << missing / (savings_rate * income) << " years" << std::endl;
    std::cout << " Time to FI (w/ returns): " << base_months / 12.0 << " years" << std::endl;
    std::cout << std::endl;

    std::array<int, 5> decs{1, 2, 5, 10, 20};

    for (auto dec : decs) {
        auto dec_savings_rate = savings_rate + 0.01 * dec;

        auto current_nw        = nw;
        size_t months = 0;

        while (current_nw < years * expenses) {
            current_nw *= 1.0 + (roi / 100.0) / 12;
            current_nw += (dec_savings_rate * income) / 12;

            ++months;
        }

        std::cout << "Increasing your Savings Rate by " << dec << "% would save you " << (base_months - months) / 12.0 << " years" << std::endl;
    }
}

void retirement_set() {
    double wrate = 4.0;
    double roi = 4.0;

    if(internal_config_contains("withdrawal_rate")){
        wrate = to_number<double>(internal_config_value("withdrawal_rate"));
    }

    if(internal_config_contains("expected_roi")){
        roi = to_number<double>(internal_config_value("expected_roi"));
    }

    edit_double(wrate, "Withdrawal Rate (%)");
    edit_double(roi, "Expected Annual Return (%)");

    // Save the configuration
    internal_config_value("withdrawal_rate") = to_string(wrate);
    internal_config_value("expected_roi") = to_string(roi);
}

} // end of anonymous namespace

void budget::retirement_module::load() {
    load_accounts();
    load_assets();
    load_expenses();
    load_earnings();
}

void budget::retirement_module::handle(std::vector<std::string>& args) {
    if (args.empty() || args.size() == 1) {
        retirement_status();
    } else {
        auto& subcommand = args[1];

        if (subcommand == "status") {
            retirement_status();
        } else if (subcommand == "set") {
            retirement_set();
            std::cout << std::endl;
            retirement_status();
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}
