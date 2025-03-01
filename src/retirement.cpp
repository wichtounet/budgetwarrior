//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <array>

#include "data_cache.hpp"
#include "retirement.hpp"
#include "assets.hpp"
#include "accounts.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "config.hpp"
#include "console.hpp"
#include "writer.hpp"
#include "incomes.hpp"

using namespace budget;

namespace {

constexpr size_t running_limit = 12;

money running_expenses(data_cache & cache, const budget::date & date = budget::local_day()){
    budget::date const end = date.previous_month().end_of_month();  // The end is the last day of the previous month
    budget::date const start = end - budget::months(running_limit); // The start is N months before the end date

    budget::money total;

    if (auto & expenses = cache.sorted_expenses(); !expenses.empty()) {
        auto it = ranges::lower_bound(expenses.begin(), expenses.end(), start, ranges::less{}, [](const auto & value) { return value.date; });

        while (it != expenses.end()) {
            if (it->date <= end) {
                total += it->amount;

                ++it;
            } else {
                break;
            }
        }
    }

    return total;
}

double running_savings_rate(data_cache & cache, const budget::date & sd = budget::local_day()){
    double savings_rate = 0.0;

    for(date_type i = 1; i <= running_limit; ++i){
        auto d = sd - budget::months(i);

        auto expenses = fold_left_auto(all_expenses_month(cache, d.year(), d.month()) | to_amount);
        auto earnings = fold_left_auto(all_earnings_month(cache, d.year(), d.month()) | to_amount);
        auto income   = get_base_income(cache, d);

        auto balance = income + earnings - expenses;
        auto local   = balance / (income + earnings);

        if(local < 0){
            local = 0;
        }

        savings_rate += local;
    }

    return savings_rate / running_limit;
}

budget::money running_income(data_cache & cache, const budget::date & sd = budget::local_day()){
    budget::money income;

    for(date_type i = 1; i <= running_limit; ++i){
        auto d = sd - budget::months(i);

        auto earnings = fold_left_auto(all_earnings_month(cache, d.year(), d.month()) | to_amount);
        income += get_base_income(cache, d) + earnings;
    }

    return income;
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
    internal_config_set("withdrawal_rate", to_string(wrate));
    internal_config_set("expected_roi", to_string(roi));
}

} // end of anonymous namespace

void budget::retirement_module::load() {
    load_accounts();
    load_assets();
    load_expenses();
    load_earnings();
}

void budget::retirement_module::handle(std::vector<std::string>& args) {
    console_writer w(std::cout);

    if (args.empty() || args.size() == 1) {
        retirement_status(w);
    } else {
        auto& subcommand = args[1];

        if (subcommand == "status") {
            retirement_status(w);
        } else if (subcommand == "set") {
            retirement_set();
            std::cout << std::endl;
            retirement_status(w);
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

double budget::fi_ratio(const budget::date & d, data_cache& cache) {
    return fixed_fi_ratio(d, cache, running_expenses(cache, d));
}

double budget::fixed_fi_ratio(const budget::date& d, data_cache& cache, const money& expenses) {
    const auto wrate = to_number<double>(internal_config_value("withdrawal_rate"));
    const auto nw    = get_fi_net_worth(d, cache);

    return fixed_fi_ratio(wrate, nw, expenses);
}

double budget::fixed_fi_ratio(double wrate, const money & nw, const money & expenses) {
    const auto years   = double(100.0 / wrate);
    const auto goal = years * expenses;

    return nw / goal;
}

void budget::retirement_status(budget::writer& w) {
    if (!w.is_web()) {
        if (!internal_config_contains("withdrawal_rate")) {
            w << "Not enough information, please configure first with retirement set" << end_of_line;
            return;
        }

        if (!internal_config_contains("expected_roi")) {
            w << "Not enough information, please configure first with retirement set" << end_of_line;
            return;
        }
    }

    auto currency       = get_default_currency();
    auto wrate          = to_number<double>(internal_config_value("withdrawal_rate"));
    auto roi            = to_number<double>(internal_config_value("expected_roi"));
    auto years          = double(100.0 / wrate);
    auto expenses       = running_expenses(w.cache);
    auto savings_rate   = running_savings_rate(w.cache);
    auto nw             = get_fi_net_worth(w.cache);
    auto missing        = years * expenses - nw;
    auto income         = running_income(w.cache);

    date_type base_months = 0;
    {
        auto current_nw = nw;
        while (current_nw < years * expenses) {
            current_nw *= 1.0 + (roi / 100.0) / 12;
            current_nw += (savings_rate * income) / 12;

            ++base_months;
        }
    }

    std::vector<std::string> columns = {};
    std::vector<std::vector<std::string>> contents;

    using namespace std::string_literals;

    // The configuration
    contents.push_back({"Withdrawal rate"s, to_string(wrate) + "%"});
    contents.push_back({"Annual Return"s, to_string(roi) + "%"});
    contents.push_back({"Years of expense"s, to_string(years)});

    // The target
    contents.push_back({""s, ""s});
    contents.push_back({"Running expenses"s, to_string(expenses) + " " + currency});
    contents.push_back({"Monthly expenses"s, to_string(expenses / 12) + " " + currency});
    contents.push_back({"Target Net Worth"s, to_string(years * expenses) + " " + currency});

    contents.push_back({""s, ""s});
    contents.push_back({"Current Net Worth"s, to_string(nw) + " " + currency});
    contents.push_back({"Missing Net Worth"s, to_string(missing) + " " + currency});
    contents.push_back({"Running Income"s, to_string(income) + " " + currency});
    contents.push_back({"Running Savings Rate"s, to_string(100 * savings_rate) + "%"});
    contents.push_back({"Yearly savings"s, to_string(savings_rate * income) + " " + currency});
    contents.push_back({"FI Ratio"s, to_string(100 * (nw / missing)) + "%"});

    auto fi_date = budget::local_day() + budget::months(base_months);
    contents.push_back({""s, ""s});
    contents.push_back({"Months to FI"s, to_string(base_months)});
    contents.push_back({"Years to FI"s, to_string(base_months / 12.0)});
    contents.push_back({"Date to FI"s, to_string(fi_date)});

    contents.push_back({""s, ""s});
    contents.push_back({"Current Withdrawal Rate"s, to_string(100.0 * (expenses / nw)) + "%"});
    contents.push_back({"Months of FI"s, to_string(nw / (expenses / 12))});
    contents.push_back({"Years of FI"s, to_string(nw / expenses)});

    contents.push_back({""s, ""s});
    contents.push_back({"Current Yearly Allowance"s, to_string(nw * (wrate / 100.0))});
    contents.push_back({"Current Monthly Allowance"s, to_string((nw * (wrate / 100.0)) / 12)});

    w.display_table(columns, contents);

    const std::array<int, 5> rate_decs{1, 2, 5, 10, 20};

    // Note: this not totally correct since we ignore the
    // correlation between the savings rate and the expenses

    for (auto dec : rate_decs) {
        auto dec_savings_rate = savings_rate + 0.01 * dec;

        auto current_nw = nw;
        size_t months   = 0;

        while (current_nw < years * expenses) {
            current_nw *= 1.0 + (roi / 100.0) / 12;
            current_nw += (dec_savings_rate * income) / 12;

            ++months;
        }

        w << p_begin << "Increasing Savings Rate by " << dec << "% would save " << double(base_months - months) / 12.0 << " years (in " << double(months) / 12.0
          << " years)" << p_end;
    }

    const std::array<int, 5> exp_decs{10, 50, 100, 200, 500};

    for (auto dec : exp_decs) {
        auto current_nw = nw;
        size_t months   = 0;

        auto new_savings_rate = (income - (expenses - dec * 12)) / income;

        while (current_nw < years * (expenses - (dec * 12))) {
            current_nw *= 1.0 + (roi / 100.0) / 12;
            current_nw += (new_savings_rate * income) / 12;

            ++months;
        }

        w << p_begin << "Decreasing monthly expenses by " << dec << " " << currency << " would save " << double(base_months - months) / 12.0 << " years (in "
          << double(months) / 12.0 << " (adjusted) years)" << p_end;
    }
}
