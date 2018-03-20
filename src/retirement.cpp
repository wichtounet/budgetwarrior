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
#include "writer.hpp"

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
    auto years          = double(int(100.0 / wrate));
    auto expenses       = running_expenses();
    auto savings_rate   = running_savings_rate();
    auto nw             = get_net_worth();
    auto missing        = years * expenses - nw;
    auto income         = 12 * get_base_income();
    auto a_savings_rate = (income - expenses) / income;

    size_t base_months   = 0;
    size_t a_base_months = 0;

    auto current_nw = nw;
    while (current_nw < years * expenses) {
        current_nw *= 1.0 + (roi / 100.0) / 12;
        current_nw += (savings_rate * income) / 12;

        ++base_months;
    }

    current_nw = nw;
    while (current_nw < years * expenses) {
        current_nw *= 1.0 + (roi / 100.0) / 12;
        current_nw += (a_savings_rate * income) / 12;

        ++a_base_months;
    }

    std::vector<std::string> columns = {};
    std::vector<std::vector<std::string>> contents;

    using namespace std::string_literals;

    contents.push_back({"Withdrawal rate"s, to_string(wrate) + "%"});
    contents.push_back({"Annual Return"s, to_string(roi) + "%"});
    contents.push_back({"Years of expense"s, to_string(years)});
    contents.push_back({"Running expenses"s, to_string(expenses) + " " + currency});
    contents.push_back({"Target Net Worth"s, to_string(years * expenses) + " " + currency});
    contents.push_back({"Current Net Worth"s, to_string(nw) + " " + currency});
    contents.push_back({"Missing Net Worth"s, to_string(missing) + " " + currency});
    contents.push_back({"FI Ratio"s, to_string(100 * (nw / missing)) + "%"});
    contents.push_back({"Yearly income"s, to_string(income) + " " + currency});
    contents.push_back({"Running Savings Rate"s, to_string(100 * savings_rate) + "%"});
    contents.push_back({"Yearly savings"s, to_string(savings_rate * income) + " " + currency});
    contents.push_back({"Time to FI (w/o returns)"s, to_string(missing / (savings_rate * income)) + " years"});
    contents.push_back({"Time to FI (w/ returns)"s, to_string(base_months / 12.0) + " years"});

    contents.push_back({"Adjusted Savings Rate"s, to_string(100 * a_savings_rate) + "%"});
    contents.push_back({"Adjusted Yearly savings"s, to_string(a_savings_rate * income) + " " + currency});
    contents.push_back({"Adjusted Time to FI (w/o returns)"s, to_string(missing / (a_savings_rate * income)) + " years"});
    contents.push_back({"Adjusted Time to FI (w/ returns)"s, to_string(a_base_months / 12.0) + " years"});

    w.display_table(columns, contents);

    std::array<int, 5> rate_decs{1, 2, 5, 10, 20};

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

        w << p_begin << "Increasing Savings Rate by " << dec << "% would save " << (base_months - months) / 12.0 << " years (in " << months / 12.0 << " years)" << p_end;
    }

    std::array<int, 5> exp_decs{10, 50, 100, 200, 500};

    for (auto dec : exp_decs) {
        auto current_nw = nw;
        size_t months   = 0;

        auto new_savings_rate = (income - (expenses - dec * 12)) / income;;

        while (current_nw < years * (expenses - (dec * 12))) {
            current_nw *= 1.0 + (roi / 100.0) / 12;
            current_nw += (new_savings_rate * income) / 12;

            ++months;
        }

        w << p_begin << "Decreasing monthly expenses by " << dec << " " << currency << " would save " << (base_months - months) / 12.0 << " years (in " << months / 12.0 << " (adjusted) years)" << p_end;
    }
}
