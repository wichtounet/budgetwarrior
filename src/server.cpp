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

    stream << R"=====(
        <nav class="navbar navbar-expand-md navbar-dark bg-dark fixed-top">
          <a class="navbar-brand" href="#">budgetwarrior</a>
          <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarsExampleDefault" aria-controls="navbarsExampleDefault" aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
          </button>

          <div class="collapse navbar-collapse" id="navbarsExampleDefault">
            <ul class="navbar-nav mr-auto">
              <li class="nav-item active">
                <a class="nav-link" href="#">Index <span class="sr-only">(current)</span></a>
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
                  <a class="dropdown-item" href="/expenses/">Expenses</a>
                  <a class="dropdown-item" href="/expenses/all/">All Expenses</a>
                </div>
              </li>
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown04" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Earnings</a>
                <div class="dropdown-menu" aria-labelledby="dropdown04">
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

void html_stream(std::stringstream& content_stream, const std::string& title){
    content_stream.imbue(std::locale("C"));

    content_stream << header(title);
}

void html_answer(std::stringstream& content_stream, httplib::Response& res){
    content_stream << footer();

    res.set_content(content_stream.str(), "text/html");
}

void index_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "");

    // TODO

    html_answer(content_stream, res);
}

void accounts_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Accounts");

    budget::html_writer w(content_stream);
    budget::show_accounts(w);

    html_answer(content_stream, res);
}

void all_accounts_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "All Accounts");

    budget::html_writer w(content_stream);
    budget::show_all_accounts(w);

    html_answer(content_stream, res);
}

void overview_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Overview");

    budget::html_writer w(content_stream);

    if(req.matches.size() == 3){
        display_month_overview(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        display_month_overview(w);
    }

    html_answer(content_stream, res);
}

void overview_aggregate_year_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Overview Aggregate");

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

    html_answer(content_stream, res);
}

void overview_aggregate_month_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Overview Aggregate");

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

    html_answer(content_stream, res);
}

void overview_year_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Overview Year");

    budget::html_writer w(content_stream);

    if(req.matches.size() == 2){
        display_year_overview(to_number<size_t>(req.matches[1]), w);
    } else {
        display_year_overview(w);
    }

    html_answer(content_stream, res);
}

void expenses_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Expenses");

    budget::html_writer w(content_stream);

    if(req.matches.size() == 3){
        show_expenses(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        show_expenses(w);
    }

    html_answer(content_stream, res);
}

void all_expenses_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "All Expenses");

    budget::html_writer w(content_stream);
    budget::show_all_expenses(w);

    html_answer(content_stream, res);
}

void earnings_page(const httplib::Request& req, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Earnings");

    budget::html_writer w(content_stream);

    if(req.matches.size() == 3){
        show_earnings(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        show_earnings(w);
    }

    html_answer(content_stream, res);
}

void all_earnings_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "All Earnings");

    budget::html_writer w(content_stream);
    budget::show_all_earnings(w);

    html_answer(content_stream, res);
}

void portfolio_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Portfolio");

    budget::html_writer w(content_stream);
    budget::show_asset_portfolio(w);

    html_answer(content_stream, res);
}

void rebalance_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Rebalance");

    budget::html_writer w(content_stream);
    budget::show_asset_rebalance(w);

    html_answer(content_stream, res);
}

void assets_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Assets");

    budget::html_writer w(content_stream);
    budget::show_assets(w);

    html_answer(content_stream, res);
}

void asset_values_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Net Worth");

    budget::html_writer w(content_stream);
    budget::show_asset_values(w);

    html_answer(content_stream, res);
}

void objectives_list_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Objectives List");

    budget::html_writer w(content_stream);
    budget::list_objectives(w);

    html_answer(content_stream, res);
}

void objectives_status_page(const httplib::Request&, httplib::Response& res){
    std::stringstream content_stream;
    html_stream(content_stream, "Objectives Status");

    budget::html_writer w(content_stream);
    budget::status_objectives(w);

    html_answer(content_stream, res);
}

} //end of anonymous namespace

void budget::server_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
    load_assets();
    load_objectives();
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

    server.get(R"(/earnings/(\d+)/(\d+)/)", &earnings_page);
    server.get("/earnings/", &earnings_page);
    server.get("/earnings/all/", &all_earnings_page);

    server.get("/portfolio/", &portfolio_page);
    server.get("/rebalance/", &rebalance_page);
    server.get("/assets/", &assets_page);
    server.get("/net_worth/", &asset_values_page);

    server.get("/objectives/list/", &objectives_list_page);
    server.get("/objectives/status/", &objectives_status_page);

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
