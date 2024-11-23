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
#include <utility>

#include "cpp_utils/assert.hpp"

#include "cpp_utils/hash.hpp"
#include "overview.hpp"
#include "console.hpp"
#include "data_cache.hpp"
#include "compute.hpp"
#include "budget_exception.hpp"
#include "config.hpp"
#include "views.hpp"
#include "writer.hpp"

using namespace budget;

namespace {

bool invalid_accounts_all(){
    data_cache cache;

    auto sy = start_year(cache);

    auto today = budget::local_day();

    const std::vector<budget::account> previous = all_accounts(cache, sy, start_month(cache, sy));

    for(budget::year year = sy; year <= today.year(); ++year){
        auto sm = start_month(cache, year);

        for(budget::month month = sm; month.is_valid(); ++month){
            auto current_accounts = all_accounts(cache, year, month);

            if(current_accounts.size() != previous.size()){
                return true;
            }

            for(const auto& c : current_accounts){
                if(!std::ranges::any_of(previous, [&c](const auto & p) { return p.name == c.name; })){
                    return true;
                }
            }
        }
    }

    return false;
}

bool invalid_accounts(budget::year year){
    data_cache cache;

    auto sm = start_month(cache, year);

    std::vector<budget::account> previous = all_accounts(cache, year, sm);

    for(budget::month m = sm + date_type(1); m.is_valid(); ++m){
        auto current_accounts = all_accounts(cache, year, m);

        if(current_accounts.size() != previous.size()){
            return true;
        }

        for(const auto& c : current_accounts){
            bool found = false;

            for(const auto& p : previous){
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

    total_line.emplace_back("");
    total_line.push_back(title);
    total_line.push_back(to_string(functor(values.front())));

    for(size_t i = 1; i < values.size(); ++i){
        total_line.emplace_back("");
        total_line.emplace_back("");
        total_line.push_back(to_string(functor(values[i])));
    }

    contents.push_back(std::move(total_line));
}

template<typename T>
void add_recap_line(std::vector<std::vector<std::string>>& contents, const std::string& title, const std::vector<T>& values){
    return add_recap_line(contents, title, values, [](const T& t){return t;});
}

budget::money compute_total_budget_account(data_cache & cache, const budget::account & account, budget::month month, budget::year year){
    // By default, the start is the year of the overview
    auto start_year_report = year;

    // Using option, can change to the beginning of all time
    if(budget::config_contains_and_true("multi_year_balance")){
        start_year_report = start_year(cache);
    }

    budget::money total;

    for(budget::year y = start_year_report; y <= year; ++y){
        budget::month m = start_month(cache, y);

        while(true){
            if(y == year && m >= month){
                break;
            }

            // Note: we still need to access the previous accounts since the
            // current account could be a more recent version of an archived
            // account
            for(const auto& prev_account : all_accounts(cache, y, m)){
                if (prev_account.name == account.name) {
                    total += prev_account.amount;
                    total -= fold_left_auto(all_expenses_month(cache, prev_account.id, y, m) | to_amount);
                    total += fold_left_auto(all_earnings_month(cache, prev_account.id, y, m) | to_amount);

                    break;
                }
            }

            if (y != year && m.is_last()) {
                break;
            }

            ++m;
        }
    }

    // Note: Here we do not strictly have to access the previous version
    // since this version is supposed to be called with match account/month/year
    // But doing so may prevent issue
    for (const auto& prev_account : all_accounts(cache, year, month)) {
        if (prev_account.name == account.name) {
            total += prev_account.amount;
            break;
        }
    }

    return total;
}

std::vector<budget::money> compute_total_budget(data_cache & cache, budget::month month, budget::year year){
    cpp::string_hash_map<budget::money> tmp;

    // By default, the start is the year of the overview
    auto start_year_report = year;

    // Using option, can change to the beginning of all time
    if(budget::config_contains_and_true("multi_year_balance")){
        start_year_report = start_year(cache);
    }

    for(budget::year y = start_year_report; y <= year; ++y){
        budget::month m = start_month(cache, y);

        while(true){
            if(y == year && m >= month){
                break;
            }

            for(const auto& account : all_accounts(cache, y, m)){
                tmp[account.name] += account.amount;
                tmp[account.name] -= fold_left_auto(all_expenses_month(cache, account.id, y, m) | to_amount);
                tmp[account.name] += fold_left_auto(all_earnings_month(cache, account.id, y, m) | to_amount);
            }

            if(y != year && m.is_last()){
                break;
            }

            ++m;
        }
    }

    std::vector<budget::money> total_budgets;

    for(const auto& account : all_accounts(cache, year, month)){
        tmp[account.name] += account.amount;

        total_budgets.push_back(tmp[account.name]);
    }

    return total_budgets;
}

template <std::ranges::range R>
void add_values_column(budget::month                          month,
                       budget::year                           year,
                       const std::string&                     title,
                       std::vector<std::vector<std::string>>& contents,
                       cpp::string_hash_map<size_t>&          indexes,
                       size_t                                 columns,
                       R&&                               values,
                       std::vector<budget::money>&            total) {
    std::vector<size_t> current(columns, contents.size());

    auto sorted_values = to_vector(std::forward<R>(values));
    std::ranges::sort(sorted_values, [](const auto& a, const auto& b) { return a.date < b.date; });

    for (auto& expense : sorted_values | filter_by_date(year, month)) {
        if (indexes.contains(get_account(expense.account).name)) {
            const size_t index = indexes[get_account(expense.account).name];
            size_t&      row   = current[index];

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

    //Totals of expenses
    contents.emplace_back(columns * 3, "");
    add_recap_line(contents, title, total);
}

using acc_data_t = cpp::string_hash_map<cpp::istring_hash_map<budget::money>>;

template<std::ranges::range R, typename Functor>
std::pair<budget::money, acc_data_t> aggregate(data_cache & cache, R && data, bool full, bool disable_groups, const std::string& separator, Functor&& func){
    budget::money total;
    acc_data_t acc_data;

    //Accumulate all the data
    for (const auto& element : std::forward<R>(data)) {
        if (func(element)) {
            auto name = element.name;

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
                acc_data["All accounts"][name] += element.amount;
            } else {
                auto account = get_account(element.account);
                acc_data[account.name][name] += element.amount;
            }

            total += element.amount;
        }
    }

    for (const auto& account : current_accounts(cache)) {
        if (!acc_data.contains(account.name)) {
            acc_data[account.name];
        }
    }

    return {total, acc_data};
}

template<std::ranges::range R, typename Functor>
void aggregate_overview(R && data, budget::writer& w, bool full, bool disable_groups, const std::string& separator, Functor&& func){
    auto [total, acc_data] = aggregate(w.cache, std::forward<R>(data), full, disable_groups, separator, func);

    cpp::string_hash_map<budget::money> totals;

    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    for (auto& account : current_accounts(w.cache)) {
        auto& items = acc_data[account.name];

        auto column = columns.size();
        columns.push_back(account.name);
        size_t row = 0;

        using s_items = std::pair<std::string, budget::money>;
        std::vector<s_items> sorted_data;

        for (auto& [name, amount] : items) {
            sorted_data.push_back(std::make_pair(name, amount));
        }

        std::ranges::sort(sorted_data,
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

    for(auto& account : current_accounts(w.cache)){
        contents.back()[i] = to_string(totals[account.name]);
        contents.back()[i + 1] = to_string_precision(100.0 * (totals[account.name] / total), 2) + "%";
        i += 3;
    }

    w.display_table(columns, contents, 3);
}

template<std::ranges::range R, typename Functor>
void aggregate_overview_month(R && data, budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year, Functor&& func){
    int months = 1;
    if (year == budget::local_day().year()) {
        months = budget::local_day().month();
    } else {
        months = 12 - budget::start_month(w.cache, year) + 1;
    }

    auto [total, acc_data] = aggregate(w.cache, std::forward<R>(data), full, disable_groups, separator, func);

    cpp::string_hash_map<budget::money> totals;

    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    for (auto& account : current_accounts(w.cache)) {
        auto& items = acc_data[account.name];

        auto column = columns.size();
        columns.push_back(account.name);
        size_t row = 0;

        using s_items = std::pair<std::string, budget::money>;
        std::vector<s_items> sorted_data;

        for (auto& [name, amount] : items) {
            sorted_data.push_back(std::make_pair(name, amount));
        }

        std::ranges::sort(sorted_data,
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

    for(auto& account : current_accounts(w.cache)){
        contents.back()[i] = to_string(totals[account.name]);
        i += 3;
    }

    w.display_table(columns, contents, 3);
}

budget::money future_value(budget::money start) {
    auto value = start;

    for (size_t y = 0; y < 10; ++y) {
        value *= 1.05;
        value += start;
    }

    return value;
}

template<std::ranges::range R, typename Functor>
void aggregate_overview_fv(R && data, budget::writer& w, bool full, bool disable_groups, const std::string& separator, Functor&& func){
    auto [total, acc_data] = aggregate(w.cache, std::forward<R>(data), full, disable_groups, separator, func);

    cpp::string_hash_map<budget::money> totals;

    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    for (auto& account : current_accounts(w.cache)) {
        auto& items = acc_data[account.name];

        auto column = columns.size();
        columns.push_back(account.name);
        size_t row = 0;

        using s_items = std::pair<std::string, budget::money>;
        std::vector<s_items> sorted_data;

        for (auto& [name, amount] : items) {
            sorted_data.push_back(std::make_pair(name, amount));
        }

        std::ranges::sort(sorted_data,
            [](const s_items& a, const s_items& b){ return a.second > b.second; });

        for (auto& [name, amount] : sorted_data) {
            if(contents.size() <= row){
                contents.emplace_back(acc_data.size() * 3, "");
            }

            contents[row][column * 3] = name;
            contents[row][column * 3 + 1] = to_string(amount);
            contents[row][column * 3 + 2] = to_string_precision(future_value(amount), 2);

            totals[account.name] += amount;

            ++row;
        }
    }

    contents.emplace_back(acc_data.size() * 3, "");
    contents.emplace_back(acc_data.size() * 3, "");

    size_t i = 0;

    contents.back()[i++] = "Total";

    for(auto& account : current_accounts(w.cache)){
        contents.back()[i] = to_string(totals[account.name]);
        i += 3;
    }

    w.display_table(columns, contents, 3);
}

void add_month_columns(std::vector<std::string>& columns, budget::month sm){
    for(budget::month m = sm; m.is_valid(); ++m){
        columns.emplace_back(m.as_long_string());
    }
}

budget::month get_current_months(data_cache & cache, budget::year year){
    auto sm = start_month(cache, year);
    budget::month current_months = budget::month(12) - sm + budget::month(1);

    if(auto today = budget::local_day(); today.year() == year){
        current_months = today.month() - sm + date_type(1);
    }

    return current_months;
}

template<bool Mean = false, bool CMean = false>
inline void generate_total_line(data_cache & cache, std::vector<std::vector<std::string>>& contents, std::vector<budget::money>& totals, budget::year year, budget::month sm){
    std::vector<std::string> last_row;
    last_row.emplace_back("Total");

    auto current_months = get_current_months(cache, year);

    budget::money total_total;
    budget::money current_total;
    for(budget::month m = sm; m.is_valid(); ++m){
        auto total = totals[m.value - 1];

        last_row.push_back(format_money(total));

        total_total += total;

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

template<std::ranges::range R>
void display_values(budget::writer& w, budget::year year, const std::string& title, R&& values, bool current = true, bool relaxed = true, bool last = false){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(w.cache, year);
    auto months = 12 - sm + 1;
    auto current_months = get_current_months(w.cache, year);

    columns.push_back(title);
    add_month_columns(columns, sm);
    columns.emplace_back("Total");
    columns.emplace_back("Mean");

    if(current){
        columns.emplace_back("C. Total");
        columns.emplace_back("C. Mean");
    }

    cpp::string_hash_map<size_t> row_mapping;
    cpp::string_hash_map<budget::money> account_totals;;
    cpp::string_hash_map<budget::money> account_current_totals;;
    std::vector<budget::money> totals(13, budget::money());

    //Prepare the rows

    for(auto& account : all_accounts(w.cache, year, sm)){
        row_mapping[account.name] = contents.size();

        contents.push_back({account.name});
    }

    //Fill the table

    for(budget::month m = sm; m.is_valid(); ++m){
        for(auto& account : all_accounts(w.cache, year, m)){
            budget::money month_total;

            if (relaxed) {
                month_total = fold_left_auto(std::forward<R>(values) | filter_by_account_name(account.name) | filter_by_date(year, m) | to_amount);
            } else {
                month_total = fold_left_auto(std::forward<R>(values) | filter_by_account(account.id) | filter_by_date(year, m) | to_amount);
            }

            contents[row_mapping[account.name]].push_back(to_string(month_total));

            account_totals[account.name] += month_total;
            totals[m.value - 1] += month_total;

            if(m < sm + current_months){
                account_current_totals[account.name] += month_total;
            }
        }
    }

    //Generate total and mean columns for each account

    for(auto& account : all_accounts(w.cache, year, sm)){
        contents[row_mapping[account.name]].push_back(to_string(account_totals[account.name]));
        contents[row_mapping[account.name]].push_back(to_string(account_totals[account.name] / months));

        if(current){
            contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name]));
            contents[row_mapping[account.name]].push_back(format_money(account_current_totals[account.name] / current_months));
        }
    }

    //Generate the final total line

    if(current){
        generate_total_line<true, true>(w.cache, contents, totals, year, sm);
    } else {
        generate_total_line<true, false>(w.cache, contents, totals, year, sm);
    }

    if(last){
        contents.push_back({"Previous Year"});

        const budget::year last_year = year - date_type(1);
        budget::money total;

        for(budget::month m = sm; m.is_valid(); ++m){
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

void budget::overview_module::load(){
    load_accounts();
    load_incomes();
    load_expenses();
    load_earnings();

    // Yearly overview needs net worth data
    load_earnings();
    load_fortunes();
    load_assets();
}

void budget::overview_module::handle(std::vector<std::string>& args) {
    if (no_accounts()) {
        throw budget_exception("No accounts defined, you should start by defining some of them");
    }

    budget::console_writer w(std::cout);

    auto today = budget::local_day();

    if (args.empty() || args.size() == 1) {
        return display_month_overview(today.month(), today.year(), w);
    }

    auto& subcommand = args[1];

    if (subcommand == "month") {
        if (args.size() == 2) {
            display_month_overview(today.month(), today.year(), w);
        } else if (args.size() == 3) {
            display_month_overview(budget::month(to_number<unsigned short>(args[2])), today.year(), w);
        } else if (args.size() == 4) {
            display_month_overview(budget::month(to_number<unsigned short>(args[2])), budget::year(to_number<unsigned short>(args[3])), w);
        } else {
            throw budget_exception("Too many arguments to overview month");
        }
    } else if (subcommand == "year") {
        if (args.size() == 2) {
            display_year_overview_header(today.year(), w);
            display_year_overview(today.year(), w);
        } else if (args.size() == 3) {
            display_year_overview_header(budget::year(to_number<unsigned short>(args[2])), w);
            display_year_overview(budget::year(to_number<unsigned short>(args[2])), w);
        } else {
            throw budget_exception("Too many arguments to overview month");
        }
    } else if (subcommand == "aggregate") {
        // Get defaults from config

        bool        full           = budget::config_contains_and_true("aggregate_full");
        bool        disable_groups = budget::config_contains_and_true("aggregate_no_group");
        std::string separator      = budget::config_value("aggregate_separator", "/");

        // Command-line  overrides config

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
                    aggregate_month_overview(w, full, disable_groups, separator, budget::month(to_number<unsigned short>(args[3])), today.year());
                } else if (args.size() == 5) {
                    aggregate_month_overview(w,
                                             full,
                                             disable_groups,
                                             separator,
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
        auto ask_for_account = [&today](budget::month m = budget::month{0}, budget::year y = budget::year{0}) {
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
            display_month_account_overview(ask_for_account(), today.month(), today.year(), w);
        } else {
            auto& subsubcommand = args[2];

            if (subsubcommand == "month") {
                if (args.size() == 3) {
                    display_month_account_overview(ask_for_account(), today.month(), today.year(), w);
                } else if (args.size() == 4) {
                    budget::month const m(to_number<unsigned short>(args[3]));
                    display_month_account_overview(ask_for_account(m), m, today.year(), w);
                } else if (args.size() == 5) {
                    budget::month const m(to_number<unsigned short>(args[3]));
                    budget::year const  y(to_number<unsigned short>(args[4]));

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

void budget::display_expenses(budget::writer& w, budget::year year, bool current, bool relaxed, bool last){
    display_values(w, year, "Expenses", all_expenses() | persistent, current, relaxed, last);
}

void budget::display_earnings(budget::writer& w, budget::year year, bool current, bool relaxed, bool last){
    display_values(w, year, "Earnings", all_earnings(), current, relaxed, last);
}

void budget::display_local_balance(budget::writer& w, budget::year year, bool current, bool relaxed, bool last){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(w.cache, year);
    auto months = 12 - sm + 1;
    auto current_months = get_current_months(w.cache, year);

    columns.emplace_back("Local Balance");
    add_month_columns(columns, sm);
    columns.emplace_back("Total");
    columns.emplace_back("Mean");

    if(current){
        columns.emplace_back("C. Total");
        columns.emplace_back("C. Mean");
    }

    std::vector<budget::money> totals(12, budget::money());

    cpp::string_hash_map<size_t> row_mapping;
    cpp::string_hash_map<budget::money> account_totals;
    cpp::string_hash_map<budget::money> account_current_totals;

    //Prepare the rows

    for(auto& account : all_accounts(w.cache, year, sm)){
        row_mapping[account.name] = contents.size();

        contents.push_back({account.name});
    }

    //Fill the table

    for(budget::month m = sm; m.is_valid(); ++m){
        for(auto& account : all_accounts(w.cache, year, m)){
            budget::money total_expenses;
            budget::money total_earnings;

            if(relaxed){
                auto relaxed_filter = [account](const auto & e){return get_account(e.account).name == account.name; };
                total_expenses = fold_left_auto(w.cache.expenses() | persistent | filter_by_date(year, m) | std::views::filter(relaxed_filter) | to_amount);
                total_earnings = fold_left_auto(w.cache.earnings() | filter_by_date(year, m) | std::views::filter(relaxed_filter) | to_amount);
            } else {
                total_expenses = fold_left_auto(all_expenses_month(w.cache, account.id, year, m) | to_amount);
                total_earnings = fold_left_auto(all_earnings_month(w.cache, account.id, year, m) | to_amount);
            }

            auto month_total = account.amount - total_expenses + total_earnings;

            contents[row_mapping[account.name]].push_back(format_money(month_total));

            account_totals[account.name] += month_total;

            totals[m.value - 1] += month_total;

            if(m < sm + current_months){
                account_current_totals[account.name] += month_total;
            }
        }
    }

    //Generate total and mean columns for each account

    for(auto& account : all_accounts(w.cache, year, sm)){
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
        generate_total_line<true, true>(w.cache, contents, totals, year, sm);
    } else {
        generate_total_line<true, false>(w.cache, contents, totals, year, sm);
    }

    if (last) {
        contents.push_back({"Previous Year"});

        budget::money total;

        for(budget::month m = sm; m.is_valid(); ++m){
            auto status = compute_month_status(w.cache, year - date_type(1), m);

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

        for (budget::month m = sm; m.is_valid(); ++m) {
            auto status = compute_month_status(w.cache, year, m);

            auto savings        = status.income - status.expenses;
            double savings_rate = 0.0;

            if (savings.dollars() > 0) {
                savings_rate = 100.0 * (savings / status.income);
            }

            contents.back().push_back(to_string_precision(savings_rate, 2) + "%");

            if (m < sm + current_months) {
                current_total_savings_rate += savings_rate;
            }

            total_savings_rate += savings_rate;
        }

        contents.back().emplace_back("");
        contents.back().push_back(to_string_precision(total_savings_rate / 12, 2) + "%");

        if (current) {
            contents.back().emplace_back("");
            contents.back().push_back(to_string_precision(current_total_savings_rate / current_months, 2) + "%");
        }
    }

    {
        contents.push_back({"Previous Year"});

        double total_savings_rate         = 0.0;
        double current_total_savings_rate = 0.0;

        for (budget::month m = sm; m.is_valid(); ++m) {
            auto status = compute_month_status(w.cache, year - date_type(1), m);

            double savings_rate = 0.0;

            if (status.balance.dollars() > 0) {
                savings_rate = 100.0 * (status.balance / (status.budget + status.earnings));
            }

            contents.back().push_back(to_string_precision(savings_rate, 2) + "%");

            if (m < sm + current_months) {
                current_total_savings_rate += savings_rate;
            }

            total_savings_rate += savings_rate;
        }

        contents.back().emplace_back("");
        contents.back().push_back(to_string_precision(total_savings_rate / 12, 2) + "%");

        if (current) {
            contents.back().emplace_back("");
            contents.back().push_back(to_string_precision(current_total_savings_rate / current_months, 2) + "%");
        }
    }

    w.display_table(columns, contents, 1, {6, 8}, 0, contents.size() - c_foot);
}

void budget::display_balance(budget::writer& w, budget::year year, bool relaxed, bool last){
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> contents;

    auto sm = start_month(w.cache, year);

    columns.emplace_back("Balance");
    add_month_columns(columns, sm);

    std::vector<budget::money> totals(12, budget::money());

    cpp::string_hash_map<size_t> row_mapping;
    cpp::string_hash_map<std::vector<budget::money>> account_previous;

    //Prepare the rows

    for(const auto& account : all_accounts(w.cache, year, sm)){
        row_mapping[account.name] = contents.size();

        contents.push_back({account.name});
        account_previous[account.name] = std::vector<budget::money>(13, budget::money());
    }

    if(auto today = budget::local_day(); year > today.year()){
        auto pretotal = compute_total_budget(w.cache, sm, year);
        for(size_t i = 0; const auto& account : all_accounts(w.cache, year, sm)){
            account_previous[account.name][sm.value - 1] += pretotal[i++] - account.amount;
        }
    }

    //Fill the table

    for(budget::month m = sm; m.is_valid(); ++m){
        for(const auto& account : all_accounts(w.cache, year, m)){
            budget::money total_expenses;
            budget::money total_earnings;

            if(relaxed){
                auto relaxed_filter = [account](const auto & e){return get_account(e.account).name == account.name; };
                total_expenses = fold_left_auto(w.cache.expenses() | persistent | filter_by_date(year, m) | std::views::filter(relaxed_filter) | to_amount);
                total_earnings = fold_left_auto(w.cache.earnings() | filter_by_date(year, m) | std::views::filter(relaxed_filter) | to_amount);
            } else {
                total_expenses = fold_left_auto(all_expenses_month(w.cache, account.id, year, m) | to_amount);
                total_earnings = fold_left_auto(all_earnings_month(w.cache, account.id, year, m) | to_amount);
            }

            auto month_total = account_previous[account.name][m.value - 1] + account.amount - total_expenses + total_earnings;
            account_previous[account.name][m.value] = month_total;

            totals[m.value - 1] += month_total;

            contents[row_mapping[account.name]].push_back(format_money(month_total));
        }
    }

    //Generate the final total line

    generate_total_line(w.cache, contents, totals, year, sm);

    if(last){
        contents.push_back({"Previous Year"});

        budget::money total;

        for(budget::month m = sm; m.is_valid(); ++m){
            auto status = compute_month_status(w.cache, year - date_type(1), m);

            total += status.balance;

            contents.back().push_back(format_money(total));
        }
    }

    w.display_table(columns, contents, 1, {}, 0, last ? 2 : 1);
}

void budget::display_month_overview(budget::month month, budget::year year, budget::writer& writer){
    auto accounts = all_accounts(writer.cache, year, month);

    writer << title_begin << "Overview of " << month << " " << year << budget::year_month_selector{"overview", year, month} << title_end;

    std::vector<std::string> columns;
    cpp::string_hash_map<size_t> indexes;
    std::vector<std::vector<std::string>> contents;
    std::vector<money> total_expenses(accounts.size(), budget::money());
    std::vector<money> total_earnings(accounts.size(), budget::money());

    for(auto& account : accounts){
        indexes[account.name] = columns.size();
        columns.push_back(account.name);
    }

    //Expenses
    add_values_column(month, year, "Expenses", contents, indexes, columns.size(), writer.cache.expenses() | persistent, total_expenses);

    //Earnings
    contents.emplace_back(columns.size() * 3, "");
    add_values_column(month, year, "Earnings", contents, indexes, columns.size(), writer.cache.earnings(), total_earnings);

    //Budget
    contents.emplace_back(columns.size() * 3, "");
    add_recap_line(contents, "Budget", accounts, [](const budget::account& a){return format_money(a.amount);});
    auto total_budgets = compute_total_budget(writer.cache, month, year);
    add_recap_line(contents, "Total Budget", total_budgets, [](const budget::money& m){ return format_money(m);});

    //Balances
    contents.emplace_back(columns.size() * 3, "");

    std::vector<budget::money> balances;
    std::vector<budget::money> local_balances;

    budget::money income = get_base_income(writer.cache, budget::date(year, month, 1));

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

    auto avg_status = budget::compute_avg_month_status(writer.cache, year, month);

    std::vector<std::string> second_columns;
    std::vector<std::vector<std::string>> second_contents;

    second_contents.emplace_back(std::vector<std::string>{"Total expenses", budget::to_string(total_all_expenses)});

    budget::money taxes;

    if (has_taxes_account()) {
       auto taxes_account_name = taxes_account().name;

       auto expenses_no_taxes = total_all_expenses - total_expenses[indexes[taxes_account_name]];
       second_contents.emplace_back(std::vector<std::string>{"Expenses w/o taxes", budget::to_string(expenses_no_taxes)});
       taxes = total_expenses[indexes[taxes_account_name]];
    }

    second_contents.emplace_back(std::vector<std::string>{"Avg expenses", budget::to_string(avg_status.expenses)});
    second_contents.emplace_back(std::vector<std::string>{"Total earnings", budget::to_string(total_all_earnings)});
    second_contents.emplace_back(std::vector<std::string>{"Avg earnings", budget::to_string(avg_status.earnings)});
    second_contents.emplace_back(std::vector<std::string>{"Income", budget::format_money(income)});
    second_contents.emplace_back(std::vector<std::string>{"Balance", budget::format_money(total_balance)});
    second_contents.emplace_back(std::vector<std::string>{"Local Balance", budget::format_money(total_local_balance)});
    second_contents.emplace_back(std::vector<std::string>{"Avg Local Balance", budget::format_money(avg_status.balance)});

    auto savings           = income - total_all_expenses;
    auto income_no_taxes   = income - taxes;
    auto expenses_no_taxes = total_all_expenses - taxes;
    auto savings_no_taxes  = income_no_taxes - expenses_no_taxes;

    double savings_rate       = 0.0;
    double savings_rate_after = 0.0;

    if (savings.positive()) {
        savings_rate = 100 * (savings / income);
    }

    if (savings_no_taxes.positive()) {
        savings_rate_after = 100 * (savings_no_taxes / income_no_taxes);
    }

    second_contents.emplace_back(std::vector<std::string>{"Savings", budget::to_string(savings)});
    second_contents.emplace_back(std::vector<std::string>{"Savings Rate", budget::to_string(savings_rate) + "%"});

    if (has_taxes_account()) {
        second_contents.emplace_back(std::vector<std::string>{"Savings Rate After Tax", budget::to_string(savings_rate_after) + "%"});

        const double tax_rate = 100 * (taxes / income);

        second_contents.emplace_back(std::vector<std::string>{"Tax Rate", budget::to_string(tax_rate) + "%"});
    }

    const budget::date month_start(year, month, 1);
    const budget::date month_end = month_start.end_of_month();

    auto net_worth_end = get_net_worth(month_end, writer.cache);
    auto net_worth_month_start = get_net_worth(month_start, writer.cache);

    auto month_increase = net_worth_end - net_worth_month_start;

    second_contents.emplace_back(std::vector<std::string>{"Net Worth Increase", budget::to_string(month_increase)});

    if (month_increase.zero() || month_increase.negative()) {
        second_contents.emplace_back(std::vector<std::string>{"Savings Contribution", "N/A"});
    } else {
        second_contents.emplace_back(std::vector<std::string>{"Savings Contribution", budget::to_string(100.0 * (savings / month_increase)) + "%"});
    }

    writer.display_table(second_columns, second_contents, 1, {}, accounts.size() * 9 + 1);
}

void budget::display_month_account_overview(size_t account_id, budget::month month, budget::year year, budget::writer& writer){
    auto account = get_account(account_id);

    auto accounts = all_accounts(writer.cache, year, month);

    writer << title_begin << "Account Overview of " << month << " " << year << budget::year_month_selector{"account_overview", year, month} << title_end;

    std::vector<std::string> columns{account.name};
    cpp::string_hash_map<size_t> indexes{{account.name, 0}};
    std::vector<std::vector<std::string>> contents;
    std::vector<money> total_expenses(1, budget::money());
    std::vector<money> total_earnings(1, budget::money());

    //Expenses
    add_values_column(month, year, "Expenses", contents, indexes, columns.size(), writer.cache.expenses() | persistent, total_expenses);

    //Earnings
    contents.emplace_back(columns.size() * 3, "");
    add_values_column(month, year, "Earnings", contents, indexes, columns.size(), writer.cache.earnings(), total_earnings);

    //Budget
    contents.emplace_back(columns.size() * 3, "");
    add_recap_line<budget::account>(contents, "Budget", {account}, [](const budget::account& a) { return format_money(a.amount); });
    auto total_budget = compute_total_budget_account(writer.cache, account, month, year);
    add_recap_line<budget::money>(contents, "Total Budget", {total_budget}, [](const budget::money& m){ return format_money(m);});

    //Balances
    contents.emplace_back(columns.size() * 3, "");

    const std::vector<budget::money> balances{total_budget + total_earnings[0] - total_expenses[0]};
    const std::vector<budget::money> local_balances{account.amount + total_earnings[0] - total_expenses[0]};

    add_recap_line(contents, "Balance", balances, [](const budget::money& m){ return format_money(m);});
    add_recap_line(contents, "Local Balance", local_balances, [](const budget::money& m){ return format_money(m);});

    writer.display_table(columns, contents, 3);
}

void budget::display_year_overview_header(budget::year year, budget::writer& w){
    if(invalid_accounts(year)){
        throw budget::budget_exception("The accounts of the different months have different names, impossible to generate the year overview. ");
    }

    w << title_begin << "Overview of " << year << budget::year_selector{"overview/year", year} << title_end;

    auto today       = budget::local_day();
    auto status      = year == today.year() ? compute_year_status(w.cache, year, today.month()) : compute_year_status(w.cache, year);
    auto prev_year   = year - date_type(1);
    auto prev_status = prev_year == today.year() ? compute_year_status(w.cache, prev_year, today.month()) : compute_year_status(w.cache, prev_year);

    std::vector<std::vector<std::string>> contents;

    contents.push_back({"Total expenses",
                        to_string(status.expenses),
                        format_money_reverse(status.expenses - prev_status.expenses),
                        format_double_reverse(100.0 * (status.expenses / prev_status.expenses - 1.0)) + "%"});

    if (!status.taxes.zero()) {
        contents.push_back({"Expenses w/o taxes",
                            to_string(status.expenses_no_taxes()),
                            format_money_reverse(status.expenses_no_taxes() - prev_status.expenses_no_taxes()),
                            format_double_reverse(100.0 * (status.expenses_no_taxes() / prev_status.expenses_no_taxes() - 1.0)) + "%"});
    }

    contents.push_back({"Total earnings",
                        to_string(status.earnings),
                        format_money(status.earnings - prev_status.earnings),
                        format_double(100 * (status.earnings / prev_status.earnings - 1.0)) + "%"});
    contents.push_back({"Total income",
                        to_string(status.income),
                        format_money(status.income - prev_status.income),
                        format_double(100.0 * (status.income / prev_status.income - 1.0)) + "%"});
    contents.push_back({"Savings",
                        to_string(status.savings),
                        format_money(status.savings - prev_status.savings),
                        format_double(100.0 * (status.savings / prev_status.savings - 1.0)) + "%"});
    contents.push_back({"Savings Rate",
                        to_string(status.savings_rate()) + "%",
                        format_double(status.savings_rate() - prev_status.savings_rate()),
                        format_double(100.0 * (status.savings_rate() / prev_status.savings_rate() - 1.0)) + "%"});
    contents.push_back({"Savings Rate After Tax",
                        to_string(status.savings_rate_after_tax()) + "%",
                        format_double(status.savings_rate_after_tax() - prev_status.savings_rate_after_tax()),
                        format_double(100.0 * (status.savings_rate_after_tax() / prev_status.savings_rate_after_tax() - 1.0)) + "%"});

    if (!status.taxes.zero()){
        contents.push_back({"Tax Rate",
                            to_string(status.tax_rate()) + "%",
                            format_double_reverse(status.tax_rate() - prev_status.tax_rate()),
                            format_double_reverse(100.0 * (status.tax_rate() / prev_status.tax_rate() - 1.0)) + "%"});
    }

    const budget::date  year_start(year, 1, 1);
    const budget::money year_increase = get_net_worth(year_start.end_of_year(), w.cache) - get_net_worth(year_start, w.cache);

    const budget::date  prev_year_start(prev_year, 1, 1);
    const budget::money prev_year_increase = get_net_worth(prev_year_start.end_of_year(), w.cache) - get_net_worth(prev_year_start, w.cache);

    contents.push_back({"Net Worth Increase",
                        to_string(year_increase),
                        format_money(year_increase - prev_year_increase),
                        format_double(100.0 * (year_increase / prev_year_increase - 1.0)) + "%"});

    if (year_increase.zero() || year_increase.negative()) {
        contents.push_back({"Savings Contribution", "N/A", "" , ""});
    } else {
        auto savings_contribution      = 100.0 * (status.savings / year_increase);
        auto prev_savings_contribution = 100.0 * (prev_status.savings / prev_year_increase);
        contents.push_back({"Savings Contribution",
                            to_string(savings_contribution) + "%",
                            format_double_reverse(savings_contribution - prev_savings_contribution),
                            format_double_reverse(100.0 * (savings_contribution / prev_savings_contribution - 1.0)) + "%"});
    }

    std::vector<std::string> columns;
    w.display_table(columns, contents);
}

void budget::display_year_overview(budget::year year, budget::writer& w){
    if(invalid_accounts(year)){
        throw budget::budget_exception("The accounts of the different months have different names, impossible to generate the year overview. ");
    }

    auto today = budget::local_day();
    const bool current = year == today.year() && today.month() != budget::month(12);

    display_local_balance(w, year, current, false, true);
    display_balance(w, year, false, true);
    display_expenses(w, year, current, false, true);
    display_earnings(w, year, current, false, true);
}

void budget::aggregate_all_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator){
    if(invalid_accounts_all()){
        throw budget::budget_exception("The accounts of the different years or months have different names, impossible to generate the complete overview. ");
    }

    w << title_begin << "Aggregate overview of all time" << title_end;

    w << p_begin << "Expenses" << p_end;
    aggregate_overview(all_expenses() | persistent, w, full, disable_groups, separator, [](const budget::expense& /*expense*/){ return true; });

    w << p_begin << "Earnings" << p_end;
    aggregate_overview(all_earnings(), w, full, disable_groups, separator, [](const budget::earning& /*earning*/){ return true; });
}

void budget::aggregate_year_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year){
    if(invalid_accounts(year)){
        throw budget::budget_exception("The accounts of the different months have different names, impossible to generate the year overview. ");
    }

    w << title_begin << "Aggregate overview of " << year << year_selector{"overview/aggregate/year", year} << title_end;

    w << p_begin << "Expenses" << p_end;
    aggregate_overview(all_expenses() | persistent, w, full, disable_groups, separator, [year](const budget::expense& expense){ return expense.date.year() == year; });

    w << p_begin << "Earnings" << p_end;
    aggregate_overview(all_earnings(), w, full, disable_groups, separator, [year](const budget::earning& earning){ return earning.date.year() == year; });
}

void budget::aggregate_year_month_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year){
    if(invalid_accounts(year)){
        throw budget::budget_exception("The accounts of the different months have different names, impossible to generate the year overview. ");
    }

    w << title_begin << "Aggregate overview of " << year << year_selector{"overview/aggregate/year_month", year} << title_end;

    w << p_begin << "Expenses" << p_end;
    aggregate_overview_month(all_expenses() | persistent, w, full, disable_groups, separator, year, [year](const budget::expense& expense){ return expense.date.year() == year; });

    w << p_begin << "Earnings" << p_end;
    aggregate_overview_month(all_earnings(), w, full, disable_groups, separator, year, [year](const budget::earning& earning){ return earning.date.year() == year; });
}

void budget::aggregate_year_fv_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::year year){
    if(invalid_accounts(year)){
        throw budget::budget_exception("The accounts of the different months have different names, impossible to generate the year overview. ");
    }

    w << title_begin << "Aggregate FV overview of " << year << year_selector{"overview/aggregate/year_fv", year} << title_end;

    w << p_begin << "Expenses" << p_end;
    aggregate_overview_fv(all_expenses() | persistent, w, full, disable_groups, separator, [year](const budget::expense& expense){ return expense.date.year() == year; });

    w << p_begin << "Earnings" << p_end;
    aggregate_overview_fv(all_earnings(), w, full, disable_groups, separator, [year](const budget::earning& earning){ return earning.date.year() == year; });
}

void budget::aggregate_month_overview(budget::writer& w, bool full, bool disable_groups, const std::string& separator, budget::month month, budget::year year){
    w << title_begin << "Aggregate overview of " << month << " " << year << year_month_selector{"overview/aggregate/month", year, month} << title_end;

    w << p_begin << "Expenses" << p_end;
    aggregate_overview(all_expenses() | persistent, w, full, disable_groups, separator, [month,year](const budget::expense& expense){ return expense.date.month() == month && expense.date.year() == year; });

    w << p_begin << "Earnings" << p_end;
    aggregate_overview(all_earnings(), w, full, disable_groups, separator, [month,year](const budget::earning& earning){ return earning.date.month() == month && earning.date.year() == year; });
}

void budget::add_expenses_column(budget::month                            month,
                                 budget::year                             year,
                                 const std::string&                       title,
                                 std::vector<std::vector<std::string>>&   contents,
                                 cpp::string_hash_map<size_t>& indexes,
                                 size_t                                   columns,
                                 const std::vector<expense>&              values,
                                 std::vector<budget::money>&              total) {
    add_values_column(month, year, title, contents, indexes, columns, values | persistent, total);
}

void budget::add_earnings_column(budget::month                            month,
                                 budget::year                             year,
                                 const std::string&                       title,
                                 std::vector<std::vector<std::string>>&   contents,
                                 cpp::string_hash_map<size_t>& indexes,
                                 size_t                                   columns,
                                 const std::vector<earning>&              values,
                                 std::vector<budget::money>&              total) {
    add_values_column(month, year, title, contents, indexes, columns, values, total);
}
