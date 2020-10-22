//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <cstring>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

#include "cpp_utils/assert.hpp"

#include "overview.hpp"
#include "console.hpp"
#include "accounts.hpp"
#include "compute.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "config.hpp"
#include "incomes.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

bool invalid_accounts_all(){
    auto sy = start_year();

    auto today = budget::local_day();

    std::vector<budget::account> previous = all_accounts(sy, start_month(sy));

    for(unsigned short j = sy; j <= today.year(); ++j){
        budget::year year = j;

        auto sm = start_month(year);

        for(unsigned short i = sm; i < 13; ++i){
            budget::month month = i;

            auto current_accounts = all_accounts(year, month);

            if(current_accounts.size() != previous.size()){
                return true;
            }

            for(auto& c : current_accounts){
                bool found = false;

                for(auto& p : previous){
                    if(p.name == c.name){
                        found = true;
                        break;
                    }
                }

                if(!found){
                    return true;
                }
            }
        }
    }

    return false;
}

bool invalid_accounts(budget::year year){
    auto sm = start_month(year);

    std::vector<budget::account> previous = all_accounts(year, sm);;

    for(unsigned short i = sm + 1; i < 13; ++i){
        budget::month month = i;

        auto current_accounts = all_accounts(year, month);

        if(current_accounts.size() != previous.size()){
            return true;
        }

        for(auto& c : current_accounts){
            bool found = false;

            for(auto& p : previous){
                if(p.name == c.name){
                    found = true;
                    break;
                }
            }

            if(!found){
                return true;
            }
        }

        previous = std::move(current_accounts);
    }

    return false;
}

template<typename T, typename J>
void add_recap_line(std::vector<std::vector<std::string>>& contents, const std::string& title, const std::vector<T>& values, J functor){
    std::vector<std::string> total_line;

    total_line.push_back("");
    total_line.push_back(title);
    total_line.push_back(to_string(functor(values.front())));

    for(size_t i = 1; i < values.size(); ++i){
        total_line.push_back("");
        total_line.push_back("");
        total_line.push_back(to_string(functor(values[i])));
    }

    contents.push_back(std::move(total_line));
}

template<typename T>
void add_recap_line(std::vector<std::vector<std::string>>& contents, const std::string& title, const std::vector<T>& values){
    return add_recap_line(contents, title, values, [](const T& t){return t;});
}

budget::money compute_total_budget_account(budget::account & account, budget::month month, budget::year year){
    // By default, the start is the year of the overview
    auto start_year_report = year;

    // Using option, can change to the beginning of all time
    if(budget::config_contains("multi_year_balance") && budget::config_value("multi_year_balance") == "true"){
        start_year_report = start_year();
    }

    budget::money total;

    for(budget::year y = start_year_report; y <= year; y = y + 1){
        budget::month m = start_month(y);

        while(true){
            if(y == year && m >= month){
                break;
            }

            // Note: we still need to access the previous accounts since the
            // current account could be a more recent version of an archived
            // account
            for(auto& prev_account : all_accounts(y, m)){
                if (prev_account.name == account.name) {
                    total += prev_account.amount;
                    total -= accumulate_amount(all_expenses_month(prev_account.id, y, m));
                    total += accumulate_amount(all_earnings_month(prev_account.id, y, m));

                    break;
                }
            }

            if (y != year && m == 12) {
                break;
            }

            m = m + 1;
        }
    }

    // Note: Here we do not strictly have to access the previous version
    // since this version is supposed to be called with match account/month/year
    // But doing so may prevent issue
    for (auto& prev_account : all_accounts(year, month)) {
        if (prev_account.name == account.name) {
            total += prev_account.amount;
            break;
        }
    }

    return total;
}

std::vector<budget::money> compute_total_budget(budget::month month, budget::year year){
    std::unordered_map<std::string, budget::money> tmp;

    // By default, the start is the year of the overview
    auto start_year_report = year;

    // Using option, can change to the beginning of all time
    if(budget::config_contains("multi_year_balance") && budget::config_value("multi_year_balance") == "true"){
        start_year_report = start_year();
    }

    for(budget::year y = start_year_report; y <= year; y = y + 1){
        budget::month m = start_month(y);

        while(true){
            if(y == year && m >= month){
                break;
            }

            for(auto& account : all_accounts(y, m)){
                tmp[account.name] += account.amount;
                tmp[account.name] -= accumulate_amount(all_expenses_month(account.id, y, m));
                tmp[account.name] += accumulate_amount(all_earnings_month(account.id, y, m));
            }

            if(y != year && m == 12){
                break;
            }

            m = m + 1;
        }
    }

    std::vector<budget::money> total_budgets;

    for(auto& account : all_accounts(year, month)){
        tmp[account.name] += account.amount;

        total_budgets.push_back(tmp[account.name]);
    }

    return total_budgets;
}

template <typename T>
void add_values_column(budget::month month, budget::year year, const std::string& title, std::vector<std::vector<std::string>>& contents,
                       std::unordered_map<std::string, size_t>& indexes, size_t columns, const std::vector<T>& values, std::vector<budget::money>& total) {
    std::vector<size_t> current(columns, contents.size());

    std::vector<T> sorted_values = values;
    std::sort(sorted_values.begin(), sorted_values.end(), [](const T& a, const T& b) { return a.date < b.date; });

    for (auto& expense : sorted_values) {
        if (expense.date.year() == year && expense.date.month() == month) {
            if (indexes.count(get_account(expense.account).name)) {
                size_t index = indexes[get_account(expense.account).name];
                size_t& row  = current[index];

                if (contents.size() <= row) {
                    contents.emplace_back(columns * 3, "");
                }

                contents[row][index * 3]     = to_string(expense.date.day());
                contents[row][index * 3 + 1] = expense.name;
                contents[row][index * 3 + 2] = to_string(expense.amount);

                total[index] += expense.amount;

                ++row;
            }
        }
    }

    //Totals of expenses
    contents.emplace_back(columns * 3, "");
    add_recap_line(contents, title, total);
}

struct icompare_str {
    bool operator()(const std::string& lhs, const std::string& rhs) const {
        // Note: This is very fast, but not very good for locale
        return strcasecmp(lhs.c_str(), rhs.c_str()) == 0;
    }

    size_t operator()(const std::string& value) const {
        auto l_value = value;
        std::transform(l_value.begin(), l_value.end(), l_value.begin(), ::tolower);
        std::hash<std::string> hasher;
        return hasher(l_value);
    }
};

template<typename Data, typename Functor>
void aggregate_overview(const Data & data, budget::writer& w, bool full, bool disable_groups, const std::string& separator, Functor&& func){
    std::unordered_map<std::string, std::unordered_map<std::string, budget::money, icompare_str, icompare_str>> acc_data;

    budget::money total;

    //Accumulate all the data
    for (auto& data : data) {
        if (func(data)) {
            auto name = data.name;

            if (name[name.size() - 1] == ' ') {
                name.erase(name.size() - 1, name.size());
            }

            if (!disable_groups) {
                auto loc = name.find(separator);
                if (loc != std::string::npos) {
                    name = name.substr(0, loc);
                }
            }

            if (full) {
                acc_data["All accounts"][name] += data.amount;
            } else {
                auto account = get_account(data.account);
                acc_data[account.name][name] += data.amount;
            }

            total += data.amount;
        }
    }

    for (auto& account : current_accounts()) {
        auto it = acc_data.find(account.name);

        if (it == acc_data.end()) {
            acc_data[account.name];
        }
    }

    std::unordered_map<std::string, budget::money> totals;

    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    for (auto& account : current_accounts()) {
        auto& items = acc_data[account.name];

        auto column = columns.size();
        columns.push_back(account.name);
        size_t row = 0;

        using s_items = std::pair<std::string, budget::money>;
        std::vector<s_items> sorted_data;

        for (auto& [name, amount] : items) {
            sorted_data.push_back(std::make_pair(name, amount));
        }

        std::sort(sorted_data.begin(), sorted_data.end(),
            [](const s_items& a, const s_items& b){ return a.second > b.second; });

        for (auto& [name, amount] : sorted_data) {
            if(contents.size() <= row){
                contents.emplace_back(acc_data.size() * 3, "");
            }

            contents[row][column * 3] = name;
            contents[row][column * 3 + 1] = to_string(amount);
            contents[row][column * 3 + 2] = to_string_precision(100.0 * (amount / total), 2) + "%";

            totals[account.name] += amount;

            ++row;
        }
    }

    contents.emplace_back(acc_data.size() * 3, "");
    contents.emplace_back(acc_data.size() * 3, "");

    size_t i = 0;

    contents.back()[i++] = "Total";

    for(auto& account : current_accounts()){
        contents.back()[i] = to_string(totals[account.name]);
        contents.back()[i + 1] = to_string_precision(100.0 * (totals[account.name] / total), 2) + "%";
        i += 3;
    }

    w.display_table(columns, contents, 3);
}

template<typename Data, typename Functor>
void aggregate_overview_month(const Data & data, budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year, Functor&& func){
    std::unordered_map<std::string, std::unordered_map<std::string, budget::money, icompare_str, icompare_str>> acc_data;

    int months;
    if (year == budget::local_day().year()) {
        months = budget::local_day().month();
    } else {
        months = 12 - budget::start_month(year) + 1;
    }

    budget::money total;

    //Accumulate all the data
    for (auto& data : data) {
        if (func(data)) {
            auto name = data.name;

            if (name[name.size() - 1] == ' ') {
                name.erase(name.size() - 1, name.size());
            }

            if (!disable_groups) {
                auto loc = name.find(separator);
                if (loc != std::string::npos) {
                    name = name.substr(0, loc);
                }
            }

            if (full) {
                acc_data["All accounts"][name] += data.amount;
            } else {
                auto account = get_account(data.account);
                acc_data[account.name][name] += data.amount;
            }

            total += data.amount;
        }
    }

    for (auto& account : current_accounts()) {
        auto it = acc_data.find(account.name);

        if (it == acc_data.end()) {
            acc_data[account.name];
        }
    }

    std::unordered_map<std::string, budget::money> totals;

    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    for (auto& account : current_accounts()) {
        auto& items = acc_data[account.name];

        auto column = columns.size();
        columns.push_back(account.name);
        size_t row = 0;

        using s_items = std::pair<std::string, budget::money>;
        std::vector<s_items> sorted_data;

        for (auto& [name, amount] : items) {
            sorted_data.push_back(std::make_pair(name, amount));
        }

        std::sort(sorted_data.begin(), sorted_data.end(),
            [](const s_items& a, const s_items& b){ return a.second > b.second; });

        for (auto& [name, amount] : sorted_data) {
            if(contents.size() <= row){
                contents.emplace_back(acc_data.size() * 3, "");
            }

            contents[row][column * 3] = name;
            contents[row][column * 3 + 1] = to_string(amount);
            contents[row][column * 3 + 2] = to_string_precision(amount / months, 2);

            totals[account.name] += amount;

            ++row;
        }
    }

    contents.emplace_back(acc_data.size() * 3, "");
    contents.emplace_back(acc_data.size() * 3, "");

    size_t i = 0;

    contents.back()[i++] = "Total";

    for(auto& account : current_accounts()){
        contents.back()[i] = to_string(totals[account.name]);
        i += 3;
    }

    w.display_table(columns, contents, 3);
}

void add_month_columns(std::vector<std::string>& columns, budget::month sm){
    for(unsigned short i = sm; i < 13; ++i){
        budget::month m = i;

        columns.emplace_back(m.as_long_string());
    }
}

int get_current_months(budget::year year){
    auto sm = start_month(year);
    auto current_months = 12 - sm + 1;

    auto today = budget::local_day();

    if(today.year() == year){
        current_months = today.month() - sm + 1;
    }

    return current_months;
}

template<bool Mean = false, bool CMean = false>
inline void generate_total_line(std::vector<std::vector<std::string>>& contents, std::vector<budget::money>& totals, budget::year year, budget::month sm){
    std::vector<std::string> last_row;
    last_row.push_back("Total");

    auto current_months = get_current_months(year);

    budget::money total_total;
    budget::money current_total;
    for(unsigned short i = sm; i < 13; ++i){
        auto total = totals[i - 1];

        last_row.push_back(format_money(total));

        total_total += total;

        budget::month m = i;
        if(m < sm + current_months){
            current_total += total;
        }
    }

    if(Mean){
        last_row.push_back(format_money(total_total));
        last_row.push_back(format_money(total_total / (12 - sm + 1)));
    }

    if(CMean){
        last_row.push_back(format_money(current_total));
        last_row.push_back(format_money(current_total / current_months));
    }

    contents.emplace_back(std::move(last_row));
}

template<typename T>
void display_values(budget::writer& w, budget::year year, const std::string& title, const std::vector<T>& values, bool current = true, bool relaxed = true, bool last = false){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(year);
    auto months = 12 - sm + 1;
    auto current_months = get_current_months(year);

    columns.push_back(title);
    add_month_columns(columns, sm);
    columns.push_back("Total");
    columns.push_back("Mean");

    if(current){
        columns.push_back("C. Total");
        columns.push_back("C. Mean");
    }

    std::unordered_map<std::string, size_t> row_mapping;
    std::unordered_map<std::string, budget::money> account_totals;;
    std::unordered_map<std::string, budget::money> account_current_totals;;
    std::vector<budget::money> totals(13, budget::money());

    //Prepare the rows

    for(auto& account : all_accounts(year, sm)){
        row_mapping[account.name] = contents.size();

        contents.push_back({account.name});
    }

    //Fill the table

    for(unsigned short j = sm; j < 13; ++j){
        budget::month m = j;

        for(auto& account : all_accounts(year, m)){
            budget::money month_total;

            for(auto& value : values){
                if(relaxed){
                    if(get_account(value.account).name == account.name && value.date.year() == year && value.date.month() == m){
                        month_total += value.amount;
                    }
                } else {
                    if(value.account == account.id && value.date.year() == year && value.date.month() == m){
                        month_total += value.amount;
                    }
                }
            }

            contents[row_mapping[account.name]].push_back(to_string(month_total));

            account_totals[account.name] += month_total;
            totals[j-1] += month_total;

            if(m < sm + current_months){
                account_current_totals[account.name] += month_total;
            }
        }
    }

    //Generate total and mean columns for each account

    for(auto& account : all_accounts(year, sm)){
        contents[row_mapping[account.name]].push_back(to_string(account_totals[account.name]));
        contents[row_mapping[account.name]].push_back(to_string(account_totals[account.name] / months));

        if(current){
            contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name]));
            contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name] / current_months));
        }
    }

    //Generate the final total line

    if(current){
        generate_total_line<true, true>(contents, totals, year, sm);
    } else {
        generate_total_line<true, false>(contents, totals, year, sm);
    }

    if(last){
        contents.push_back({"Previous Year"});

        budget::year last_year = year - 1;
        budget::money total;

        for(unsigned short j = sm; j < 13; ++j){
            budget::month m = j;

            budget::money month_total;

            for(auto& value : values){
                if(value.date.year() == last_year && value.date.month() == m){
                    month_total += value.amount;
                }
            }

            contents.back().push_back(to_string(month_total));

            total += month_total;
        }

        contents.back().push_back(to_string(total));
        contents.back().push_back(to_string(total / 12));

        if(current){
            contents.back().push_back(to_string(total));
            contents.back().push_back(to_string(total / 12));
        }
    }

    w.display_table(columns, contents, 1, {}, 0, last ? 2 : 1);
}

} // end of anonymous namespace

constexpr const std::array<std::pair<const char*, const char*>, 1> budget::module_traits<budget::overview_module>::aliases;

void budget::overview_module::load(){
    load_accounts();
    load_incomes();
    load_expenses();
    load_earnings();
}

void budget::overview_module::handle(std::vector<std::string>& args) {
    if (all_accounts().empty()) {
        throw budget_exception("No accounts defined, you should start by defining some of them");
    }

    budget::console_writer w(std::cout);

    if (args.empty() || args.size() == 1) {
        display_month_overview(w);
    } else {
        auto& subcommand = args[1];

        auto today = budget::local_day();

        if (subcommand == "month") {
            if (args.size() == 2) {
                display_month_overview(w);
            } else if (args.size() == 3) {
                display_month_overview(budget::month(to_number<unsigned short>(args[2])), w);
            } else if (args.size() == 4) {
                display_month_overview(
                    budget::month(to_number<unsigned short>(args[2])),
                    budget::year(to_number<unsigned short>(args[3])),
                    w);
            } else {
                throw budget_exception("Too many arguments to overview month");
            }
        } else if (subcommand == "year") {
            if (args.size() == 2) {
                display_year_overview(w);
            } else if (args.size() == 3) {
                display_year_overview(budget::year(to_number<unsigned short>(args[2])), w);
            } else {
                throw budget_exception("Too many arguments to overview month");
            }
        } else if (subcommand == "aggregate") {
            //Default values
            bool full             = false;
            bool disable_groups   = false;
            std::string separator = "/";

            //Get defaults from config

            if (budget::config_contains("aggregate_full")) {
                if (budget::config_value("aggregate_full") == "true") {
                    full = true;
                }
            }

            if (budget::config_contains("aggregate_no_group")) {
                if (budget::config_value("aggregate_no_group") == "true") {
                    disable_groups = true;
                }
            }

            if (budget::config_contains("aggregate_separator")) {
                separator = budget::config_value("aggregate_separator");
            }

            //Command-line  overrides config

            if (budget::option("--full", args)) {
                full = true;
            }

            if (budget::option("--no-group", args)) {
                disable_groups = true;
            }

            auto sep_value = budget::option_value("--separator", args, "");
            if (!sep_value.empty()) {
                separator = sep_value;
            }

            if (args.size() == 2) {
                aggregate_year_overview(w, full, disable_groups, separator, today.year());
            } else if (args.size() == 3 || args.size() == 4 || args.size() == 5) {
                if (args[2] == "year") {
                    if (args.size() == 3) {
                        aggregate_year_overview(w, full, disable_groups, separator, today.year());
                    } else if (args.size() == 4) {
                        aggregate_year_overview(w, full, disable_groups, separator, budget::year(to_number<unsigned short>(args[3])));
                    }
                } else if (args[2] == "month") {
                    if (args.size() == 3) {
                        aggregate_month_overview(w, full, disable_groups, separator, today.month(), today.year());
                    } else if (args.size() == 4) {
                        aggregate_month_overview(w, full, disable_groups, separator,
                                                 budget::month(to_number<unsigned short>(args[3])),
                                                 today.year());
                    } else if (args.size() == 5) {
                        aggregate_month_overview(w, full, disable_groups, separator,
                                                 budget::month(to_number<unsigned short>(args[3])),
                                                 budget::year(to_number<unsigned short>(args[4])));
                    }
                } else if (args[2] == "all") {
                    aggregate_all_overview(w, full, disable_groups, separator);
                } else {
                    throw budget_exception("Invalid subcommand");
                }
            } else {
                throw budget_exception("Too many arguments to overview aggregate");
            }
        } else if (subcommand == "account") {
            auto ask_for_account = [](budget::month m = {0}, budget::year y = {0}) {
                auto today = budget::local_day();

                if (m.is_default()) {
                    m = today.month();
                }

                if (y.is_default()) {
                    y = today.year();
                }

                std::string account_name;
                edit_string_complete(account_name, "Account", all_account_names(), not_empty_checker(), account_checker());
                return get_account(account_name, y, m).id;
            };

            if (args.size() == 2) {
                display_month_account_overview(ask_for_account(), w);
            } else {
                auto& subsubcommand = args[2];

                if (subsubcommand == "month") {
                    if (args.size() == 3) {
                        display_month_account_overview(ask_for_account(), w);
                    } else if (args.size() == 4) {
                        budget::month m(to_number<unsigned short>(args[3]));
                        display_month_account_overview(ask_for_account(m), m, w);
                    } else if (args.size() == 5) {
                        budget::month m(to_number<unsigned short>(args[3]));
                        budget::year y(to_number<unsigned short>(args[4]));

                        display_month_account_overview(ask_for_account(m, y), m, y, w);
                    } else {
                        throw budget_exception("Too many arguments to overview month");
                    }
                } else {
                    throw budget_exception("Invalid command");
                }
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::display_expenses(budget::writer& w, budget::year year, bool current, bool relaxed, bool last){
    display_values(w, year, "Expenses", all_expenses(), current, relaxed, last);
}

void budget::display_earnings(budget::writer& w, budget::year year, bool current, bool relaxed, bool last){
    display_values(w, year, "Earnings", all_earnings(), current, relaxed, last);
}

void budget::display_local_balance(budget::writer& w, budget::year year, bool current, bool relaxed, bool last){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(year);
    auto months = 12 - sm + 1;
    auto current_months = get_current_months(year);

    columns.push_back("Local Balance");
    add_month_columns(columns, sm);
    columns.push_back("Total");
    columns.push_back("Mean");

    if(current){
        columns.push_back("C. Total");
        columns.push_back("C. Mean");
    }

    std::vector<budget::money> totals(12, budget::money());

    std::unordered_map<std::string, size_t> row_mapping;
    std::unordered_map<std::string, budget::money> account_totals;
    std::unordered_map<std::string, budget::money> account_current_totals;

    //Prepare the rows

    for(auto& account : all_accounts(year, sm)){
        row_mapping[account.name] = contents.size();

        contents.push_back({account.name});
    }

    //Fill the table

    for(unsigned short i = sm; i < 13; ++i){
        budget::month m = i;

        for(auto& account : all_accounts(year, m)){
            budget::money total_expenses;
            budget::money total_earnings;

            if(relaxed){
                total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](const budget::expense& e){return get_account(e.account).name == account.name && e.date.year() == year && e.date.month() == m;});
                total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](const budget::earning& e){return get_account(e.account).name == account.name && e.date.year() == year && e.date.month() == m;});
            } else {
                total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](const budget::expense& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
                total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](const budget::earning& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
            }

            auto month_total = account.amount - total_expenses + total_earnings;

            contents[row_mapping[account.name]].push_back(format_money(month_total));

            account_totals[account.name] += month_total;

            totals[i - 1] += month_total;

            if(m < sm + current_months){
                account_current_totals[account.name] += month_total;
            }
        }
    }

    //Generate total and mean columns for each account

    for(auto& account : all_accounts(year, sm)){
        contents[row_mapping[account.name]].push_back(format_money(account_totals[account.name]));
        contents[row_mapping[account.name]].push_back(format_money(account_totals[account.name] / months));

        if(current){
            contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name]));
            contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name] / current_months));
        }
    }

    auto c_foot = contents.size();

    //Generate the total final line

    if(current){
        generate_total_line<true, true>(contents, totals, year, sm);
    } else {
        generate_total_line<true, false>(contents, totals, year, sm);
    }

    if (last) {
        contents.push_back({"Previous Year"});

        budget::money total;

        for (unsigned short i = sm; i < 13; ++i) {
            budget::month m = i;

            auto status = compute_month_status(year - 1, m);

            contents.back().push_back(format_money(status.balance));

            total += status.balance;
        }

        contents.back().push_back(format_money(total));
        contents.back().push_back(format_money(total / 12));

        if (current) {
            contents.back().push_back(format_money(total));
            contents.back().push_back(format_money(total / 12));
        }
    }

    {
        contents.push_back({"Savings Rate"});

        double total_savings_rate         = 0.0;
        double current_total_savings_rate = 0.0;

        for (unsigned short i = sm; i < 13; ++i) {
            budget::month m = i;

            auto status = compute_month_status(year, m);

            auto savings        = status.income - status.expenses;
            double savings_rate = 0.0;

            if (savings.dollars() > 0) {
                savings_rate = 100.0 * (savings / status.income);
            }

            contents.back().push_back(to_string_precision(savings_rate, 2) + "%");

            if (i < sm + current_months) {
                current_total_savings_rate += savings_rate;
            }

            total_savings_rate += savings_rate;
        }

        contents.back().push_back("");
        contents.back().push_back(to_string_precision(total_savings_rate / 12, 2) + "%");

        if (current) {
            contents.back().push_back("");
            contents.back().push_back(to_string_precision(current_total_savings_rate / current_months, 2) + "%");
        }
    }

    {
        contents.push_back({"Previous Year"});

        double total_savings_rate         = 0.0;
        double current_total_savings_rate = 0.0;

        for (unsigned short i = sm; i < 13; ++i) {
            budget::month m = i;

            auto status = compute_month_status(year - 1, m);

            double savings_rate = 0.0;

            if (status.balance.dollars() > 0) {
                savings_rate = 100.0 * (status.balance.dollars() / double((status.budget + status.earnings).dollars()));
            }

            contents.back().push_back(to_string_precision(savings_rate, 2) + "%");

            if (i < sm + current_months) {
                current_total_savings_rate += savings_rate;
            }

            total_savings_rate += savings_rate;
        }

        contents.back().push_back("");
        contents.back().push_back(to_string_precision(total_savings_rate / 12, 2) + "%");

        if (current) {
            contents.back().push_back("");
            contents.back().push_back(to_string_precision(current_total_savings_rate / current_months, 2) + "%");
        }
    }

    w.display_table(columns, contents, 1, {6, 8}, 0, contents.size() - c_foot);
}

void budget::display_balance(budget::writer& w, budget::year year, bool relaxed, bool last){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(year);

    columns.push_back("Balance");
    add_month_columns(columns, sm);

    std::vector<budget::money> totals(12, budget::money());

    std::unordered_map<std::string, size_t> row_mapping;
    std::unordered_map<std::string, std::vector<budget::money>> account_previous;

    //Prepare the rows

    for(auto& account : all_accounts(year, sm)){
        row_mapping[account.name] = contents.size();

        contents.push_back({account.name});
        account_previous[account.name] = std::vector<budget::money>(13, budget::money());
    }

    auto today = budget::local_day();
    if(year > today.year()){
        auto pretotal = compute_total_budget(sm, year);
        size_t i = 0;
        for(auto& account : all_accounts(year, sm)){
            account_previous[account.name][sm - 1] += pretotal[i++] - account.amount;
        }
    }

    //Fill the table

    for(unsigned short i = sm; i <= 12; ++i){
        budget::month m = i;

        for(auto& account : all_accounts(year, m)){
            budget::money total_expenses;
            budget::money total_earnings;

            if(relaxed){
                total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](const budget::expense& e){return get_account(e.account).name == account.name && e.date.year() == year && e.date.month() == m;});
                total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](const budget::earning& e){return get_account(e.account).name == account.name && e.date.year() == year && e.date.month() == m;});
            } else {
                total_expenses = accumulate_amount_if(all_expenses(), [account,year,m](const budget::expense& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
                total_earnings = accumulate_amount_if(all_earnings(), [account,year,m](const budget::earning& e){return e.account == account.id && e.date.year() == year && e.date.month() == m;});
            }

            auto month_total = account_previous[account.name][i - 1] + account.amount - total_expenses + total_earnings;
            account_previous[account.name][i] = month_total;

            totals[i - 1] += month_total;

            contents[row_mapping[account.name]].push_back(format_money(month_total));
        }
    }

    //Generate the final total line

    generate_total_line(contents, totals, year, sm);

    if(last){
        contents.push_back({"Previous Year"});

        budget::money total;

        for(unsigned short i = sm; i < 13; ++i){
            budget::month m = i;

            auto status = compute_month_status(year - 1, m);

            total += status.balance;

            contents.back().push_back(format_money(total));
        }
    }

    w.display_table(columns, contents, 1, {}, 0, last ? 2 : 1);
}

void budget::display_month_overview(budget::month month, budget::year year, budget::writer& writer){
    auto accounts = all_accounts(year, month);

    writer << title_begin << "Overview of " << month << " " << year << budget::year_month_selector{"overview", year, month} << title_end;

    std::vector<std::string> columns;
    std::unordered_map<std::string, size_t> indexes;
    std::vector<std::vector<std::string>> contents;
    std::vector<money> total_expenses(accounts.size(), budget::money());
    std::vector<money> total_earnings(accounts.size(), budget::money());

    for(auto& account : accounts){
        indexes[account.name] = columns.size();
        columns.push_back(account.name);
    }

    //Expenses
    add_values_column(month, year, "Expenses", contents, indexes, columns.size(), all_expenses(), total_expenses);

    //Earnings
    contents.emplace_back(columns.size() * 3, "");
    add_values_column(month, year, "Earnings", contents, indexes, columns.size(), all_earnings(), total_earnings);

    //Budget
    contents.emplace_back(columns.size() * 3, "");
    add_recap_line(contents, "Budget", accounts, [](const budget::account& a){return format_money(a.amount);});
    auto total_budgets = compute_total_budget(month, year);
    add_recap_line(contents, "Total Budget", total_budgets, [](const budget::money& m){ return format_money(m);});

    //Balances
    contents.emplace_back(columns.size() * 3, "");

    std::vector<budget::money> balances;
    std::vector<budget::money> local_balances;

    budget::money income = get_base_income(budget::date(year, month, 1));

    for(size_t i = 0; i < accounts.size(); ++i){
        balances.push_back(total_budgets[i] - total_expenses[i] + total_earnings[i]);
        local_balances.push_back(accounts[i].amount - total_expenses[i] + total_earnings[i]);

        income += total_earnings[i];
    }

    add_recap_line(contents, "Balance", balances, [](const budget::money& m){ return format_money(m);});
    add_recap_line(contents, "Local Balance", local_balances, [](const budget::money& m){ return format_money(m);});

    writer.display_table(columns, contents, 3);

    auto total_all_expenses = std::accumulate(total_expenses.begin(), total_expenses.end(), budget::money());
    auto total_all_earnings = std::accumulate(total_earnings.begin(), total_earnings.end(), budget::money());
    auto total_balance = std::accumulate(balances.begin(), balances.end(), budget::money());
    auto total_local_balance = std::accumulate(local_balances.begin(), local_balances.end(), budget::money());

    budget::money savings = income - total_all_expenses;
    double savings_rate = 0.0;

    if (savings.value > 0) {
        savings_rate = 100 * (savings / income);
    }

    auto avg_status = budget::compute_avg_month_status(year, month);

    std::vector<std::string> second_columns;
    std::vector<std::vector<std::string>> second_contents;

    second_contents.emplace_back(std::vector<std::string>{"Total expenses", budget::to_string(total_all_expenses)});

    if (config_contains("taxes_account")) {
       auto taxes_account = config_value("taxes_account");

       if (account_exists(taxes_account)) {
           auto expenses_no_taxes = total_all_expenses - total_expenses[indexes[taxes_account]];
           second_contents.emplace_back(std::vector<std::string>{"Expenses w/o taxes", budget::to_string(expenses_no_taxes)});
       }
    }

    second_contents.emplace_back(std::vector<std::string>{"Avg expenses", budget::to_string(avg_status.expenses)});
    second_contents.emplace_back(std::vector<std::string>{"Total earnings", budget::to_string(total_all_earnings)});
    second_contents.emplace_back(std::vector<std::string>{"Avg earnings", budget::to_string(avg_status.earnings)});
    second_contents.emplace_back(std::vector<std::string>{"Balance", budget::format_money(total_balance)});
    second_contents.emplace_back(std::vector<std::string>{"Local Balance", budget::format_money(total_local_balance)});
    second_contents.emplace_back(std::vector<std::string>{"Avg Local Balance", budget::format_money(avg_status.balance)});
    second_contents.emplace_back(std::vector<std::string>{"Savings", budget::to_string(savings)});
    second_contents.emplace_back(std::vector<std::string>{"Savings Rate", budget::to_string(savings_rate) + "%"});

    if (config_contains("taxes_account")) {
       auto taxes_account = config_value("taxes_account");

       if (account_exists(taxes_account)) {
           auto taxes = total_expenses[indexes[taxes_account]];

            double tax_rate = 100 * (taxes / income);

           second_contents.emplace_back(std::vector<std::string>{"Tax Rate", budget::to_string(tax_rate) + "%"});
       }
    }

    writer.display_table(second_columns, second_contents, 1, {}, accounts.size() * 9 + 1);
}

void budget::display_month_overview(budget::month month, budget::writer& writer){
    auto today = budget::local_day();

    display_month_overview(month, today.year(), writer);
}

void budget::display_month_overview(budget::writer& writer){
    auto today = budget::local_day();

    display_month_overview(today.month(), today.year(), writer);
}

void budget::display_month_account_overview(size_t account_id, budget::month month, budget::year year, budget::writer& writer){
    auto account = get_account(account_id);

    auto accounts = all_accounts(year, month);

    writer << title_begin << "Account Overview of " << month << " " << year << budget::year_month_selector{"account_overview", year, month} << title_end;

    std::vector<std::string> columns{account.name};
    std::unordered_map<std::string, size_t> indexes{{account.name, 0}};
    std::vector<std::vector<std::string>> contents;
    std::vector<money> total_expenses(1, budget::money());
    std::vector<money> total_earnings(1, budget::money());

    //Expenses
    add_values_column(month, year, "Expenses", contents, indexes, columns.size(), all_expenses(), total_expenses);

    //Earnings
    contents.emplace_back(columns.size() * 3, "");
    add_values_column(month, year, "Earnings", contents, indexes, columns.size(), all_earnings(), total_earnings);

    //Budget
    contents.emplace_back(columns.size() * 3, "");
    add_recap_line<budget::account>(contents, "Budget", {account}, [](const budget::account& a) { return format_money(a.amount); });
    auto total_budget = compute_total_budget_account(account, month, year);
    add_recap_line<budget::money>(contents, "Total Budget", {total_budget}, [](const budget::money& m){ return format_money(m);});

    //Balances
    contents.emplace_back(columns.size() * 3, "");

    std::vector<budget::money> balances{total_budget + total_earnings[0] - total_expenses[0]};
    std::vector<budget::money> local_balances{account.amount + total_earnings[0] - total_expenses[0]};

    add_recap_line(contents, "Balance", balances, [](const budget::money& m){ return format_money(m);});
    add_recap_line(contents, "Local Balance", local_balances, [](const budget::money& m){ return format_money(m);});

    writer.display_table(columns, contents, 3);
}

void budget::display_month_account_overview(size_t account_id, budget::month month, budget::writer& writer){
    auto today = budget::local_day();

    display_month_account_overview(account_id, month, today.year(), writer);
}

void budget::display_month_account_overview(size_t account_id, budget::writer& writer){
    auto today = budget::local_day();

    display_month_account_overview(account_id, today.month(), today.year(), writer);
}

void budget::display_side_month_overview(budget::month month, budget::year year, budget::writer& writer){
    auto accounts = all_accounts(year, month);

    writer << title_begin << "Side Hustle Overview of " << month << " " << year << budget::year_month_selector{"side_hustle/overview", year, month} << title_end;

    auto side_category = config_value("side_category");
    auto side_prefix   = config_value("side_prefix");

    std::vector<std::vector<std::string>> contents;
    std::vector<money> total_expenses(1, budget::money());
    std::vector<money> total_earnings(1, budget::money());

    std::vector<std::string> columns = {side_category};
    std::unordered_map<std::string, size_t> indexes = {{side_category, 0}};

    std::vector<budget::expense> side_expenses;
    std::vector<budget::earning> side_earnings;

    for (auto& expense : all_expenses()) {
        if (get_account(expense.account).name == side_category) {
            if (expense.name.find(side_prefix) == 0) {
                side_expenses.push_back(expense);
            }
        }
    }

    for (auto& earning : all_earnings()) {
        if (get_account(earning.account).name == side_category) {
            if (earning.name.find(side_prefix) == 0) {
                side_earnings.push_back(earning);
            }
        }
    }

    //Expenses
    add_values_column(month, year, "Expenses", contents, indexes, columns.size(), side_expenses, total_expenses);

    //Earnings
    contents.emplace_back(columns.size() * 3, "");
    add_values_column(month, year, "Earnings", contents, indexes, columns.size(), side_earnings, total_earnings);

    writer.display_table(columns, contents, 3);

    auto income = total_earnings[0];
    auto total_all_expenses = total_expenses[0];

    budget::money savings = income - total_all_expenses;
    double savings_rate = 0.0;

    if (savings.value > 0) {
        savings_rate = 100 * (savings / income);
    }

    std::vector<std::string> second_columns;
    std::vector<std::vector<std::string>> second_contents;

    second_contents.emplace_back(std::vector<std::string>{"Total expenses", budget::to_string(total_all_expenses)});
    second_contents.emplace_back(std::vector<std::string>{"Total earnings", budget::to_string(income)});
    second_contents.emplace_back(std::vector<std::string>{"Savings", budget::to_string(savings)});
    second_contents.emplace_back(std::vector<std::string>{"Savings Rate", budget::to_string(savings_rate) + "%"});

    writer.display_table(second_columns, second_contents, 1, {}, accounts.size() * 9 + 1);
}

void budget::display_side_month_overview(budget::month month, budget::writer& writer){
    auto today = budget::local_day();

    display_side_month_overview(month, today.year(), writer);
}

void budget::display_side_month_overview(budget::writer& writer){
    auto today = budget::local_day();

    display_side_month_overview(today.month(), today.year(), writer);
}

void budget::display_year_overview(budget::year year, budget::writer& w){
    if(invalid_accounts(year)){
        throw budget::budget_exception("The accounts of the different months have different names, impossible to generate the year overview. ");
    }

    w << title_begin << "Overview of " << year << budget::year_selector{"overview/year", year} << title_end;

    auto today = budget::local_day();
    bool current = year == today.year() && today.month() != 12;

    display_local_balance(w, year, current, false, true);
    display_balance(w, year, false, true);
    display_expenses(w, year, current, false, true);
    display_earnings(w, year, current, false, true);
}

void budget::display_year_overview(budget::writer& w){
    auto today = budget::local_day();

    display_year_overview(today.year(), w);
}

void budget::aggregate_all_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator){
    if(invalid_accounts_all()){
        throw budget::budget_exception("The accounts of the different years or months have different names, impossible to generate the complete overview. ");
    }

    w << title_begin << "Aggregate overview of all time" << title_end;

    w << p_begin << "Expenses" << p_end;
    aggregate_overview(all_expenses(), w, full, disable_groups, separator, [](const budget::expense& /*expense*/){ return true; });

    w << p_begin << "Earnings" << p_end;
    aggregate_overview(all_earnings(), w, full, disable_groups, separator, [](const budget::earning& /*earning*/){ return true; });
}

void budget::aggregate_year_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year){
    if(invalid_accounts(year)){
        throw budget::budget_exception("The accounts of the different months have different names, impossible to generate the year overview. ");
    }

    w << title_begin << "Aggregate overview of " << year << year_selector{"overview/aggregate/year", year} << title_end;

    w << p_begin << "Expenses" << p_end;
    aggregate_overview(all_expenses(), w, full, disable_groups, separator, [year](const budget::expense& expense){ return expense.date.year() == year; });

    w << p_begin << "Earnings" << p_end;
    aggregate_overview(all_earnings(), w, full, disable_groups, separator, [year](const budget::earning& earning){ return earning.date.year() == year; });
}

void budget::aggregate_year_month_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year){
    if(invalid_accounts(year)){
        throw budget::budget_exception("The accounts of the different months have different names, impossible to generate the year overview. ");
    }

    w << title_begin << "Aggregate overview of " << year << year_selector{"overview/aggregate/year_month", year} << title_end;

    w << p_begin << "Expenses" << p_end;
    aggregate_overview_month(all_expenses(), w, full, disable_groups, separator, year, [year](const budget::expense& expense){ return expense.date.year() == year; });

    w << p_begin << "Earnings" << p_end;
    aggregate_overview_month(all_earnings(), w, full, disable_groups, separator, year, [year](const budget::earning& earning){ return earning.date.year() == year; });
}

void budget::aggregate_month_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::month month, budget::year year){
    w << title_begin << "Aggregate overview of " << month << " " << year << year_month_selector{"overview/aggregate/month", year, month} << title_end;

    w << p_begin << "Expenses" << p_end;
    aggregate_overview(all_expenses(), w, full, disable_groups, separator, [month,year](const budget::expense& expense){ return expense.date.month() == month && expense.date.year() == year; });

    w << p_begin << "Earnings" << p_end;
    aggregate_overview(all_earnings(), w, full, disable_groups, separator, [month,year](const budget::earning& earning){ return earning.date.month() == month && earning.date.year() == year; });
}
