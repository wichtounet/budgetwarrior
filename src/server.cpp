//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "server.hpp"
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
#include "guid.hpp"

#include "httplib.h"

using namespace budget;

namespace {

static constexpr const char new_line = '\n';

std::string header(const std::string& title){
    std::stringstream stream;

    // The header

    stream << R"=====(
        <!doctype html>
        <html lang="en">
          <head>
            <meta charset="utf-8">
            <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
            <meta name="description" content="budgetwarrior">
            <meta name="author" content="Baptiste Wicht">
            <link href="https://getbootstrap.com/dist/css/bootstrap.min.css" rel="stylesheet">
            <style>
                body {
                  padding-top: 5rem;
                }

                p {
                    margin-bottom: 8px;
                }

                .small-form-inline {
                    float: left;
                    padding-right: 10px;
                }

                .small-text {
                    font-size: 10pt;
                }

                .extend-only {
                    width: 75%;
                }

                .selector {
                    float: right;
                    font-size: xx-large;
                    pading-top: 5px;
                }
            </style>
    )=====";

    if (title.empty()) {
        stream << "<title>budgetwarrior</title>" << new_line;
    } else {
        stream << "<title>budgetwarrior - " << title << "</title>" << new_line;
    }

    stream << "</head>" << new_line;
    stream << "<body>" << new_line;

    // The navigation

    stream << R"=====(<nav class="navbar navbar-expand-md navbar-dark bg-dark fixed-top">)=====";

    stream << "<a class=\"navbar-brand\" href=\"#\">" << budget::get_version() << "</a>";

    stream << R"=====(
          <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarsExampleDefault" aria-controls="navbarsExampleDefault" aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
          </button>
          <div class="collapse navbar-collapse" id="navbarsExampleDefault">
            <ul class="navbar-nav mr-auto">
              <li class="nav-item">
                <a class="nav-link" href="/">Index <span class="sr-only">(current)</span></a>
              </li>
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown01" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Overview</a>
                <div class="dropdown-menu" aria-labelledby="dropdown01">
                  <a class="dropdown-item" href="/overview/">Overvie Month</a>
                  <a class="dropdown-item" href="/overview/year/">Overview Year</a>
                  <a class="dropdown-item" href="/overview/aggregate/year/">Aggregate Year</a>
                  <a class="dropdown-item" href="/overview/aggregate/month/">Aggregate Month</a>
                </div>
              </li>
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown02" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Accounts</a>
                <div class="dropdown-menu" aria-labelledby="dropdown02">
                  <a class="dropdown-item" href="/accounts/">Accounts</a>
                  <a class="dropdown-item" href="/accounts/all/">All Accounts</a>
                </div>
              </li>
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown03" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Expenses</a>
                <div class="dropdown-menu" aria-labelledby="dropdown03">
                  <a class="dropdown-item" href="/expenses/add/">Add Expense</a>
                  <a class="dropdown-item" href="/expenses/">Expenses</a>
                  <a class="dropdown-item" href="/expenses/all/">All Expenses</a>
                </div>
              </li>
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown04" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Earnings</a>
                <div class="dropdown-menu" aria-labelledby="dropdown04">
                  <a class="dropdown-item" href="/earnings/add/">Add Earning</a>
                  <a class="dropdown-item" href="/earnings/">Earnings</a>
                  <a class="dropdown-item" href="/earnings/all/">All Earnings</a>
                </div>
              </li>
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown05" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Assets</a>
                <div class="dropdown-menu" aria-labelledby="dropdown05">
                  <a class="dropdown-item" href="/assets/">Assets</a>
                  <a class="dropdown-item" href="/net_worth/">Net worth</a>
                  <a class="dropdown-item" href="/portfolio/">Portfolio</a>
                  <a class="dropdown-item" href="/rebalance/">Rebalance</a>
                </div>
              </li>
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown06" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Objectives</a>
                <div class="dropdown-menu" aria-labelledby="dropdown06">
                  <a class="dropdown-item" href="/objectives/status/">Status</a>
                  <a class="dropdown-item" href="/objectives/list/">List</a>
                </div>
              </li>
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown06" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Wishes</a>
                <div class="dropdown-menu" aria-labelledby="dropdown06">
                  <a class="dropdown-item" href="/wishes/status/">Status</a>
                  <a class="dropdown-item" href="/wishes/list/">List</a>
                  <a class="dropdown-item" href="/wishes/estimate/">Estimate</a>
                </div>
              </li>
            </ul>
          </div>
        </nav>
    )=====";

    // The main component

    stream << R"=====(<main role="main" class="container-fluid">)=====" << new_line;
    stream << "<div>" << new_line;

    return stream.str();
}

std::string footer(){
    std::stringstream stream;

    stream << R"=====(
        </div>
        </main>
        <script src="https://code.jquery.com/jquery-3.2.1.slim.min.js" integrity="sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN" crossorigin="anonymous"></script>
        <script>window.jQuery || document.write('<script src="https://getbootstrap.com/assets/js/vendor/jquery.min.js"><\/script>')</script>
        <script src="https://getbootstrap.com/assets/js/vendor/popper.min.js"></script>
        <script src="https://getbootstrap.com/dist/js/bootstrap.min.js"></script>
        </body>
        </html>
    )=====";

    return stream.str();
}

void display_error_message(budget::writer& w, const std::string& message){
    w << R"=====(<div class="alert alert-danger" role="alert">)=====";
    w << message;
    w << R"=====(</div>)=====";
}

void display_message(budget::writer& w, const httplib::Request& req){
    if (req.has_param("message")) {
        if (req.has_param("error")) {
            w << R"=====(<div class="alert alert-danger" role="alert">)=====";
        } else if (req.has_param("success")) {
            w << R"=====(<div class="alert alert-success" role="alert">)=====";
        } else {
            w << R"=====(<div class="alert alert-primary" role="alert">)=====";
        }

        w << req.params.at("message");
        w << R"=====(</div>)=====";
    }
}

void html_stream(const httplib::Request& req, std::stringstream& content_stream, const std::string& title){
    content_stream.imbue(std::locale("C"));

    content_stream << header(title);

    budget::html_writer w(content_stream);
    display_message(w, req);
}

void replace_all(std::string& source, const std::string& from, const std::string& to){
    size_t current_pos = 0;

    while ((current_pos = source.find(from, current_pos)) != std::string::npos) {
        source.replace(current_pos, from.length(), to);
        current_pos += to.length();
    }
}

void filter_html(std::string& html, const httplib::Request& req){
    replace_all(html, "__budget_this_page__", req.path);
}

void html_answer(std::stringstream& content_stream, const httplib::Request& req, httplib::Response& res){
    content_stream << footer();

    auto result = content_stream.str();

    filter_html(result, req);

    res.set_content(result, "text/html");
}

void index_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "");

    budget::html_writer w(content_stream);

    w << title_begin << "Summary" << title_end;

    auto today = budget::local_day();

    // First display overview of the accounts
    budget::account_summary(w, today.month(), today.year());

    // Second display a summary of the objectives
    budget::objectives_summary(w);

    // Third display a summary of the fortune
    budget::fortune_summary(w);

    html_answer(content_stream, req, res);
}

void accounts_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Accounts");

    budget::html_writer w(content_stream);
    budget::show_accounts(w);

    html_answer(content_stream, req, res);
}

void all_accounts_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "All Accounts");

    budget::html_writer w(content_stream);
    budget::show_all_accounts(w);

    html_answer(content_stream, req, res);
}

void overview_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Overview");

    budget::html_writer w(content_stream);

    if(req.matches.size() == 3){
        display_month_overview(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        display_month_overview(w);
    }

    html_answer(content_stream, req, res);
}

void overview_aggregate_year_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Overview Aggregate");

    budget::html_writer w(content_stream);

    //Default values
    bool full = false;
    bool disable_groups = false;
    std::string separator = "/";

    //Get defaults from config

    if(budget::config_contains("aggregate_full")){
        if(budget::config_value("aggregate_full") == "true"){
            full = true;
        }
    }

    if(budget::config_contains("aggregate_no_group")){
        if(budget::config_value("aggregate_no_group") == "true"){
            disable_groups = true;
        }
    }

    if(budget::config_contains("aggregate_separator")){
        separator = budget::config_value("aggregate_separator");
    }

    if(req.matches.size() == 2){
        aggregate_year_overview(w, full, disable_groups, separator, to_number<size_t>(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_year_overview(w, full, disable_groups, separator, today.year());
    }

    html_answer(content_stream, req, res);
}

void overview_aggregate_month_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Overview Aggregate");

    budget::html_writer w(content_stream);

    //Default values
    bool full = false;
    bool disable_groups = false;
    std::string separator = "/";

    //Get defaults from config

    if(budget::config_contains("aggregate_full")){
        if(budget::config_value("aggregate_full") == "true"){
            full = true;
        }
    }

    if(budget::config_contains("aggregate_no_group")){
        if(budget::config_value("aggregate_no_group") == "true"){
            disable_groups = true;
        }
    }

    if(budget::config_contains("aggregate_separator")){
        separator = budget::config_value("aggregate_separator");
    }

    if(req.matches.size() == 3){
        aggregate_month_overview(w, full, disable_groups, separator, to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_month_overview(w, full, disable_groups, separator, today.month(), today.year());
    }

    html_answer(content_stream, req, res);
}

void overview_year_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Overview Year");

    budget::html_writer w(content_stream);

    if(req.matches.size() == 2){
        display_year_overview(to_number<size_t>(req.matches[1]), w);
    } else {
        display_year_overview(w);
    }

    html_answer(content_stream, req, res);
}

void expenses_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Expenses");

    budget::html_writer w(content_stream);

    if(req.matches.size() == 3){
        show_expenses(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        show_expenses(w);
    }

    html_answer(content_stream, req, res);
}

void all_expenses_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "All Expenses");

    budget::html_writer w(content_stream);
    budget::show_all_expenses(w);

    html_answer(content_stream, req, res);
}

void add_date_picker(budget::writer& w, const std::string& default_value = "") {
    auto today = budget::local_day();

    w << R"=====(
        <div class="form-group">
            <label for="input_date">Date</label>
            <input type="date" class="form-control" id="input_date" name="input_date" value=")=====";

    if (default_value.empty()) {
        w << today.year() << "-";

        if (today.month() < 10) {
            w << "0" << today.month().value << "-";
        } else {
            w << today.month().value << "-";
        }

        if (today.day() < 10) {
            w << "0" << today.day();
        } else {
            w << today.day();
        }
    } else {
        w << default_value;
    }

    w << R"=====(">
         </div>
    )=====";
}

void add_name_picker(budget::writer& w, const std::string& default_value = "") {
    w << R"=====(
        <div class="form-group">
            <label for="input_name">Name</label>
            <input type="text" class="form-control" id="input_name" name="input_name"
    )=====";

    if (default_value.empty()) {
        w << " placeholder=\"Enter Name\" ";
    } else {
        w << " value=\"" << default_value << "\" ";
    }

    w << R"=====(
            >
         </div>
    )=====";
}

void add_amount_picker(budget::writer& w, const std::string& default_value = "") {
    w << R"=====(
        <div class="form-group">
            <label for="input_amount">Amount</label>
            <input type="text" class="form-control" id="input_amount" name="input_amount"
    )=====";

    if (default_value.empty()) {
        w << " placeholder=\"Enter amount\" ";
    } else {
        w << " value=\"" << default_value << "\" ";
    }

    w << R"=====(
            >
         </div>
    )=====";
}

void add_account_picker(budget::writer& w, const std::string& default_value = "") {
    auto today = budget::local_day();

    w << R"=====(
            <div class="form-group">
                <label for="input_account">Account</label>
                <select class="form-control" id="input_account" name="input_account">
    )=====";

    for(auto& account : all_accounts(today.year(), today.month())){
        if (budget::to_string(account.id) == default_value) {
            w << "<option selected value=\"" << account.id << "\">" << account.name << "</option>";
        } else {
            w << "<option value=\"" << account.id << "\">" << account.name << "</option>";
        }
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void form_begin(budget::writer& w, const std::string& action, const std::string& back_page){
    w << R"=====(<form method="POST" action=")=====";
    w << action;
    w << R"=====(">)=====";
    w << R"=====(<input type="hidden" name="server" value="yes">)=====";
    w << R"=====(<input type="hidden" name="back_page" value=")=====";
    w << back_page;
    w << R"=====(">)=====";
}

void form_begin_edit(budget::writer& w, const std::string& action, const std::string& back_page, const std::string& input_id){
    form_begin(w, action, back_page);

    w << R"=====(<input type="hidden" name="input_id" value=")=====";
    w << input_id;
    w << R"=====(">)=====";
}

void form_end(budget::writer& w){
    w << R"=====(<button type="submit" class="btn btn-primary">Submit</button>)=====";
    w << "</form>";
}

void add_expenses_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    html_stream(req, content_stream, "New Expense");

    budget::html_writer w(content_stream);

    w << title_begin << "New Expense" << title_end;

    form_begin(w, "/api/expenses/add/", "/expenses/add/");

    add_date_picker(w);
    add_name_picker(w);
    add_amount_picker(w);
    add_account_picker(w);

    form_end(w);

    html_answer(content_stream, req, res);
}

void edit_expenses_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    html_stream(req, content_stream, "New Expense");

    budget::html_writer w(content_stream);

    if(!req.has_param("input_id") || !req.has_param("back_page")){
        display_error_message(w, "Invalid parameter for the request");
    } else {
        auto input_id  = req.params.at("input_id");

        if(!expense_exists(budget::to_number<size_t>(input_id))){
            display_error_message(w, "The expense " + input_id + " does not exist");
        } else {
            auto back_page = req.params.at("back_page");

            w << title_begin << "Edit Expense " << input_id << title_end;

            form_begin_edit(w, "/api/expenses/edit/", back_page, input_id);

            auto& expense = expense_get(budget::to_number<size_t>(input_id));

            add_date_picker(w, budget::to_string(expense.date));
            add_name_picker(w, expense.name);
            add_amount_picker(w, budget::to_string(expense.amount));
            add_account_picker(w, budget::to_string(expense.account));

            form_end(w);
        }
    }

    html_answer(content_stream, req, res);
}

void add_earnings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    html_stream(req, content_stream, "New earning");

    budget::html_writer w(content_stream);

    w << title_begin << "New earning" << title_end;

    form_begin(w, "/api/earnings/add/", "/earnings/add");

    add_date_picker(w);
    add_name_picker(w);
    add_amount_picker(w);
    add_account_picker(w);

    form_end(w);

    html_answer(content_stream, req, res);
}

void edit_earnings_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    html_stream(req, content_stream, "New earning");

    budget::html_writer w(content_stream);

    if(!req.has_param("input_id") || !req.has_param("back_page")){
        display_error_message(w, "Invalid parameter for the request");
    } else {
        auto input_id  = req.params.at("input_id");

        if(!earning_exists(budget::to_number<size_t>(input_id))){
            display_error_message(w, "The earning " + input_id + " does not exist");
        } else {
            auto back_page = req.params.at("back_page");

            w << title_begin << "Edit earning " << input_id << title_end;

            form_begin_edit(w, "/api/earnings/edit/", back_page, input_id);

            auto& earning = earning_get(budget::to_number<size_t>(input_id));

            add_date_picker(w, budget::to_string(earning.date));
            add_name_picker(w, earning.name);
            add_amount_picker(w, budget::to_string(earning.amount));
            add_account_picker(w, budget::to_string(earning.account));

            form_end(w);
        }
    }

    html_answer(content_stream, req, res);
}

void earnings_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Earnings");

    budget::html_writer w(content_stream);

    if(req.matches.size() == 3){
        show_earnings(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        show_earnings(w);
    }

    html_answer(content_stream, req, res);
}

void all_earnings_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "All Earnings");

    budget::html_writer w(content_stream);
    budget::show_all_earnings(w);

    html_answer(content_stream, req, res);
}

void portfolio_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Portfolio");

    budget::html_writer w(content_stream);
    budget::show_asset_portfolio(w);

    html_answer(content_stream, req, res);
}

void rebalance_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Rebalance");

    budget::html_writer w(content_stream);
    budget::show_asset_rebalance(w);

    html_answer(content_stream, req, res);
}

void assets_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Assets");

    budget::html_writer w(content_stream);
    budget::show_assets(w);

    html_answer(content_stream, req, res);
}

void asset_values_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Net Worth");

    budget::html_writer w(content_stream);
    budget::show_asset_values(w);

    html_answer(content_stream, req, res);
}

void objectives_list_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Objectives List");

    budget::html_writer w(content_stream);
    budget::list_objectives(w);

    html_answer(content_stream, req, res);
}

void objectives_status_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Objectives Status");

    budget::html_writer w(content_stream);
    budget::status_objectives(w);

    html_answer(content_stream, req, res);
}

void wishes_list_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Objectives List");

    budget::html_writer w(content_stream);
    budget::list_wishes(w);

    html_answer(content_stream, req, res);
}

void wishes_status_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Objectives Status");

    budget::html_writer w(content_stream);
    budget::status_wishes(w);

    html_answer(content_stream, req, res);
}

void wishes_estimate_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(req, content_stream, "Objectives Status");

    budget::html_writer w(content_stream);
    budget::estimate_wishes(w);

    html_answer(content_stream, req, res);
}

void api_success(const httplib::Request& req, httplib::Response& res, const std::string& message){
    if (req.has_param("server")) {
        auto url = req.params.at("back_page") + "?success=true&message=" + httplib::detail::encode_url(message);
        res.set_redirect(url.c_str());
    } else {
        res.set_content("Success: " + message, "text/plain");
    }
}

void api_error(const httplib::Request& req, httplib::Response& res, const std::string& message){
    if (req.has_param("server")) {
        auto url = req.params.at("back_page") + "?error=true&message=" + httplib::detail::encode_url(message);
        res.set_redirect(url.c_str());
    } else {
        res.set_content("Error: " + message, "text/plain");
    }
}

void add_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_name") || !req.has_param("input_date") || !req.has_param("input_amount") || !req.has_param("input_account")) {
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

    api_success(req, res, "Expense " + to_string(expense.id) + " has been created");
}

void edit_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_date") || !req.has_param("input_amount") || !req.has_param("input_account")) {
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
    if (!req.has_param("input_id")) {
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

void add_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_name") || !req.has_param("input_date") || !req.has_param("input_amount") || !req.has_param("input_account")) {
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

    api_success(req, res, "Earning " + to_string(earning.id) + " has been created");
}

void edit_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_date") || !req.has_param("input_amount") || !req.has_param("input_account")) {
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
    if (!req.has_param("input_id")) {
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

} //end of anonymous namespace

void budget::server_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
    load_assets();
    load_objectives();
    load_wishes();
    load_fortunes();
}

void budget::server_module::handle(const std::vector<std::string>& args){
    cpp_unused(args);

    std::cout << "Starting the server" << std::endl;

    httplib::Server server;

    // Declare all the pages
    server.get("/", &index_page);

    server.get("/overview/year/", &overview_year_page);
    server.get(R"(/overview/year/(\d+)/)", &overview_year_page);

    server.get("/overview/", &overview_page);
    server.get(R"(/overview/(\d+)/(\d+)/)", &overview_page);

    server.get("/overview/aggregate/year/", &overview_aggregate_year_page);
    server.get(R"(/overview/aggregate/year/(\d+)/)", &overview_aggregate_year_page);

    server.get("/overview/aggregate/month/", &overview_aggregate_month_page);
    server.get(R"(/overview/aggregate/month/(\d+)/(\d+)/)", &overview_aggregate_month_page);

    server.get("/accounts/", &accounts_page);
    server.get("/accounts/all/", &all_accounts_page);

    server.get(R"(/expenses/(\d+)/(\d+)/)", &expenses_page);
    server.get("/expenses/", &expenses_page);
    server.get("/expenses/all/", &all_expenses_page);
    server.get("/expenses/add/", &add_expenses_page);
    server.post("/expenses/edit/", &edit_expenses_page);

    server.get(R"(/earnings/(\d+)/(\d+)/)", &earnings_page);
    server.get("/earnings/", &earnings_page);
    server.get("/earnings/all/", &all_earnings_page);
    server.get("/earnings/add/", &add_earnings_page);
    server.post("/earnings/edit/", &edit_earnings_page);

    server.get("/portfolio/", &portfolio_page);
    server.get("/rebalance/", &rebalance_page);
    server.get("/assets/", &assets_page);
    server.get("/net_worth/", &asset_values_page);

    server.get("/objectives/list/", &objectives_list_page);
    server.get("/objectives/status/", &objectives_status_page);

    server.get("/wishes/list/", &wishes_list_page);
    server.get("/wishes/status/", &wishes_status_page);
    server.get("/wishes/estimate/", &wishes_estimate_page);

    // The API

    server.post("/api/expenses/add/", &add_expenses_api);
    server.post("/api/expenses/edit/", &edit_expenses_api);
    server.post("/api/expenses/delete/", &delete_expenses_api);

    server.post("/api/earnings/add/", &add_earnings_api);
    server.post("/api/earnings/edit/", &edit_earnings_api);
    server.post("/api/earnings/delete/", &delete_earnings_api);

    server.set_error_handler([](const auto&, auto& res) {
        std::stringstream content_stream;
        content_stream.imbue(std::locale("C"));

        content_stream << header("");

        content_stream << "<p>Error Status: <span style='color:red;'>";
        content_stream << res.status;
        content_stream << "</span></p>";

        content_stream << footer();

        res.set_content(content_stream.str(), "text/html");
    });


    // Listen
    server.listen("localhost", 8080);
}
