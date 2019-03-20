//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>
#include <numeric>

#include "cpp_utils/assert.hpp"

#include "accounts.hpp"
#include "assets.hpp"
#include "config.hpp"
#include "debts.hpp"
#include "expenses.hpp"
#include "objectives.hpp"
#include "overview.hpp"
#include "report.hpp"
#include "summary.hpp"
#include "version.hpp"
#include "writer.hpp"
#include "currency.hpp"

#include "pages/server_pages.hpp"
#include "http.hpp"

using namespace budget;

namespace {

static constexpr const char new_line = '\n';

std::string header(const std::string& title, bool menu = true) {
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

            <!-- The CSS -->

            <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/4.0.0-beta.3/css/bootstrap.min.css" integrity="sha256-PCsx7lOyGhyGmzsO5MGXhzwV6UpNTlNf1p6V6w2CppQ=" crossorigin="anonymous" />

            <style>
                body {
                  padding-top: 5rem;
                }

                p {
                    margin-bottom: 8px;
                }

                .asset_group {
                    margin-left: -20px;
                    margin-right: -20px;
                    padding-left: 5px;
                    border-bottom: 1px solid #343a40;
                    font-weight: bold;
                    color: #343a40;
                }

                .asset_row {
                    padding-top: 3px;
                }

                .asset_row:not(:last-child) {
                    border-bottom: 1px solid rgba(0,0,0,0.125);
                }

                .asset_name {
                    font-weight: bold;
                    color: #007bff;
                    padding-left: 5px;
                }

                .asset_right {
                    padding-left: 0px;
                    padding-right: 5px;
                }

                .asset_date {
                    color: rgba(0,0,0,0.5);
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

                .selector a {
                    font-size: xx-large;
                }

                .selector select {
                    vertical-align: middle;
                    margin-bottom: 22px;
                    margin-left: 2px;
                    margin-right: 2px;
                }

                .card {
                    margin-bottom: 10px !important;
                }

                .card-header-primary {
                    color:white !important;
                    background-color: #007bff !important;
                    padding: 0.5rem 0.75rem !important;
                }

                .gauge-cash-flow-title {
                    margin-top: -15px;
                }

                .gauge-objective-title {
                    color: rgb(124, 181, 236);
                    margin-top: -15px;
                    text-align: center;
                }

                .default-graph-style {
                    min-width: 300px;
                    height: 400px;
                    margin: 0 auto;
                }

                .dataTables_wrapper {
                    padding-left: 0px !important;
                    padding-right: 0px !important;
                }

                .flat-hr {
                    margin:0px;
                }
            </style>
    )=====";

    if (title.empty()) {
        stream << "<title>budgetwarrior</title>";
    } else {
        stream << "<title>budgetwarrior - " << title << "</title>";
    }

    stream << new_line;

    stream << "</head>" << new_line;
    stream << "<body>" << new_line;

    // The navigation

    stream << R"=====(<nav class="navbar navbar-expand-md navbar-dark bg-dark fixed-top">)=====";

    stream << "<a class=\"navbar-brand\" href=\"#\">" << budget::get_version() << "</a>";

    if (menu) {
        stream << R"=====(
          <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarsExampleDefault" aria-controls="navbarsExampleDefault" aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
          </button>
          <div class="collapse navbar-collapse" id="navbarsExampleDefault">
            <ul class="navbar-nav mr-auto">
              <li class="nav-item">
                <a class="nav-link" href="/">Index <span class="sr-only">(current)</span></a>
              </li>
        )=====";

        // Overview

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown01" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Overview</a>
                <div class="dropdown-menu" aria-labelledby="dropdown01">
                  <a class="dropdown-item" href="/overview/">Overview Month</a>
                  <a class="dropdown-item" href="/overview/year/">Overview Year</a>
                  <a class="dropdown-item" href="/overview/aggregate/year/">Aggregate Year</a>
                  <a class="dropdown-item" href="/overview/aggregate/month/">Aggregate Month</a>
                  <a class="dropdown-item" href="/overview/aggregate/all/">Aggregate All</a>
                  <a class="dropdown-item" href="/report/">Report</a>
                  <a class="dropdown-item" href="/overview/savings/time/">Savings rate over time</a>
                </div>
              </li>
        )=====";

        // Assets

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown02" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Assets</a>
                <div class="dropdown-menu" aria-labelledby="dropdown02">
                  <a class="dropdown-item" href="/assets/">Assets</a>
                  <a class="dropdown-item" href="/net_worth/status/">Net worth Status</a>
                  <a class="dropdown-item" href="/net_worth/graph/">Net worth Graph</a>
                  <a class="dropdown-item" href="/net_worth/allocation/">Net worth Allocation</a>
                  <a class="dropdown-item" href="/net_worth/currency/">Net worth Currency</a>
                  <a class="dropdown-item" href="/portfolio/status/">Portfolio Status</a>
                  <a class="dropdown-item" href="/portfolio/graph/">Portfolio Graph</a>
                  <a class="dropdown-item" href="/portfolio/allocation/">Portfolio Allocation</a>
                  <a class="dropdown-item" href="/portfolio/currency/">Portfolio Currency</a>
                  <a class="dropdown-item" href="/rebalance/">Rebalance</a>
                  <a class="dropdown-item" href="/assets/add/">Add Asset</a>
                  <a class="dropdown-item" href="/asset_values/list/">Asset Values</a>
                  <a class="dropdown-item" href="/asset_values/batch/full/">Full Batch Update</a>
                  <a class="dropdown-item" href="/asset_values/batch/current/">Current Batch Update</a>
                  <a class="dropdown-item" href="/asset_values/add/">Set One Asset Value</a>
                </div>
              </li>
        )=====";

        // Expenses

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown03" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Expenses</a>
                <div class="dropdown-menu" aria-labelledby="dropdown03">
                  <a class="dropdown-item" href="/expenses/add/">Add Expense</a>
                  <a class="dropdown-item" href="/expenses/">Expenses</a>
                  <a class="dropdown-item" href="/expenses/search/">Search</a>
                  <a class="dropdown-item" href="/expenses/all/">All Expenses</a>
                  <a class="dropdown-item" href="/expenses/breakdown/month/">Expenses Breakdown Month</a>
                  <a class="dropdown-item" href="/expenses/breakdown/year/">Expenses Breakdown Year</a>
                  <a class="dropdown-item" href="/expenses/time/">Expenses over time</a>
                </div>
              </li>
        )=====";

        // Earnings

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown04" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Earnings</a>
                <div class="dropdown-menu" aria-labelledby="dropdown04">
                  <a class="dropdown-item" href="/earnings/add/">Add Earning</a>
                  <a class="dropdown-item" href="/earnings/">Earnings</a>
                  <a class="dropdown-item" href="/earnings/all/">All Earnings</a>
                  <a class="dropdown-item" href="/earnings/time/">Earnings over time</a>
                  <a class="dropdown-item" href="/income/time/">Income over time</a>
                </div>
              </li>
        )=====";

        // Accounts

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown05" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Accounts</a>
                <div class="dropdown-menu" aria-labelledby="dropdown05">
                  <a class="dropdown-item" href="/accounts/">Accounts</a>
                  <a class="dropdown-item" href="/accounts/all/">All Accounts</a>
                  <a class="dropdown-item" href="/accounts/add/">Add Account</a>
                  <a class="dropdown-item" href="/accounts/archive/month/">Archive Account (month)</a>
                  <a class="dropdown-item" href="/accounts/archive/year/">Archive Account (year)</a>
                </div>
              </li>
        )=====";

        // Retirement

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown_retirement" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Retirement</a>
                <div class="dropdown-menu" aria-labelledby="dropdown_retirement">
                  <a class="dropdown-item" href="/retirement/status/">Status</a>
                  <a class="dropdown-item" href="/retirement/configure/">Configure</a>
                  <a class="dropdown-item" href="/retirement/fi/">FI Ratio Over Time</a>
                </div>
              </li>
        )=====";

        // Fortune

        if(!budget::is_fortune_disabled()){
            stream << R"=====(
                  <li class="nav-item dropdown">
                    <a class="nav-link dropdown-toggle" href="#" id="dropdown06" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Fortune</a>
                    <div class="dropdown-menu" aria-labelledby="dropdown06">
                      <a class="dropdown-item" href="/fortunes/graph/">Fortune</a>
                      <a class="dropdown-item" href="/fortunes/status/">Status</a>
                      <a class="dropdown-item" href="/fortunes/list/">List</a>
                      <a class="dropdown-item" href="/fortunes/add/">Set fortune</a>
                    </div>
                  </li>
            )=====";
        }

        // Objectives

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown07" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Objectives</a>
                <div class="dropdown-menu" aria-labelledby="dropdown07">
                  <a class="dropdown-item" href="/objectives/status/">Status</a>
                  <a class="dropdown-item" href="/objectives/list/">List</a>
                  <a class="dropdown-item" href="/objectives/add/">Add Objective</a>
                </div>
              </li>
        )=====";

        // Wishes

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown08" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Wishes</a>
                <div class="dropdown-menu" aria-labelledby="dropdown08">
                  <a class="dropdown-item" href="/wishes/status/">Status</a>
                  <a class="dropdown-item" href="/wishes/list/">List</a>
                  <a class="dropdown-item" href="/wishes/estimate/">Estimate</a>
                  <a class="dropdown-item" href="/wishes/add/">Add Wish</a>
                </div>
              </li>
        )=====";

        // Others

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown_others" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Others</a>
                <div class="dropdown-menu" aria-labelledby="dropdown_others">
                  <a class="dropdown-item" href="/recurrings/list/">List Recurrings</a>
                  <a class="dropdown-item" href="/recurrings/add/">Add Recurring Expense</a>
                  <div class="dropdown-divider"></div>
                  <a class="dropdown-item" href="/debts/add/">Add Debt</a>
                  <a class="dropdown-item" href="/debts/list/">List Debts</a>
                  <a class="dropdown-item" href="/debts/all/">All Debts</a>
                </div>
              </li>
        )=====";

        // Finish the menu
        stream << R"=====(
            </ul>
          </div>
        )=====";
    }

    stream << "</nav>" << new_line;

    // The main component

    stream << R"=====(<main class="container-fluid">)=====" << new_line;
    //stream << "<div>" << new_line;

    return stream.str();
}

void display_message(budget::writer& w, const httplib::Request& req) {
    if (req.has_param("message")) {
        if (req.has_param("error")) {
            w << R"=====(<div class="alert alert-danger" role="alert">)=====";
        } else if (req.has_param("success")) {
            w << R"=====(<div class="alert alert-success" role="alert">)=====";
        } else {
            w << R"=====(<div class="alert alert-primary" role="alert">)=====";
        }

        w << req.get_param_value("message");
        w << R"=====(</div>)=====";
    }
}

void replace_all(std::string& source, const std::string& from, const std::string& to) {
    size_t current_pos = 0;

    while ((current_pos = source.find(from, current_pos)) != std::string::npos) {
        source.replace(current_pos, from.length(), to);
        current_pos += to.length();
    }
}

void filter_html(std::string& html, const httplib::Request& req) {
    replace_all(html, "__budget_this_page__", req.path);
    replace_all(html, "__currency__", get_default_currency());
}

//Note: This must be synchronized with page_end
std::string footer() {
    return "</main></body></html>";
}

budget::money monthly_income(budget::month month, budget::year year) {
    std::map<size_t, budget::money> account_sum;

    for (auto& earning : all_earnings()) {
        if (earning.date.year() == year && earning.date.month() == month) {
            account_sum[earning.account] += earning.amount;
        }
    }

    budget::money total = get_base_income();

    for (auto& sum : account_sum) {
        total += sum.second;
    }

    return total;
}

budget::money monthly_spending(budget::month month, budget::year year) {
    std::map<size_t, budget::money> account_sum;

    for (auto& expense : all_expenses()) {
        if (expense.date.year() == year && expense.date.month() == month) {
            account_sum[expense.account] += expense.amount;
        }
    }

    budget::money total;

    for (auto& sum : account_sum) {
        total += sum.second;
    }

    return total;
}

void cash_flow_card(budget::html_writer& w){
    const auto today = budget::local_day();

    const auto m = today.month();
    const auto y = today.year();

    w << R"=====(<div class="card">)=====";

    auto income = monthly_income(m, y);
    auto spending = monthly_spending(m, y);

    w << R"=====(<div class="card-header card-header-primary">)=====";
    w << R"=====(<div class="float-left">Cash Flow</div>)=====";
    w << R"=====(<div class="float-right">)=====";
    w << income - spending << " __currency__";

    if(income > spending){
        w << " (" << 100.0f * ((income - spending) / income) << "%)";
    }

    w << R"=====(</div>)=====";
    w << R"=====(<div class="clearfix"></div>)=====";
    w << R"=====(</div>)====="; // card-header

    w << R"=====(<div class="row card-body">)=====";

    w << R"=====(<div class="col-md-6 col-sm-12">)=====";
    month_breakdown_income_graph(w, "Income", m, y, true, "min-width:300px; height: 300px;");
    w << R"=====(</div>)====="; //column

    w << R"=====(<div class="col-md-6 col-sm-12">)=====";
    month_breakdown_expenses_graph(w, "Expenses", m, y, true, "min-width:300px; height: 300px;");
    w << R"=====(</div>)====="; //column

    w << R"=====(</div>)====="; //card-body
    w << R"=====(</div>)====="; //card
}

void index_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "")) {
        return;
    }

    budget::html_writer w(content_stream);

    bool left_column = !all_assets().empty() && !all_asset_values().empty();

    if (left_column) {
        // A. The left column

        w << R"=====(<div class="row">)=====";

        w << R"=====(<div class="col-lg-4 d-none d-lg-block">)====="; // left column

        assets_card(w);

        w << R"=====(</div>)====="; // left column

        // B. The right column

        w << R"=====(<div class="col-lg-8 col-md-12">)====="; // right column
    }

    // 1. Display the net worth graph
    net_worth_graph(w, "min-width: 300px; width: 100%; height: 300px;", true);

    // 2. Cash flow
    cash_flow_card(w);

    // 3. Display the objectives status
    objectives_card(w);

    if (left_column) {
        w << R"=====(</div>)====="; // right column

        w << R"=====(</div>)====="; // row
    }

    // end the page
    page_end(w, req, res);
}

void report_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Report")) {
        return;
    }

    budget::html_writer w(content_stream);

    auto today = budget::local_day();
    report(w, today.year(), false, "");

    page_end(w, req, res);
}

bool parameters_present(const httplib::Request& req, std::vector<const char*> parameters) {
    for (auto& param : parameters) {
        if (!req.has_param(param)) {
            return false;
        }
    }

    return true;
}

bool validate_parameters(std::stringstream& content_stream, const httplib::Request& req, std::vector<const char*> parameters) {
    if(!parameters_present(req, parameters)){
        budget::html_writer w(content_stream);

        display_error_message(w, "Invalid parameter for the request");

        return false;
    }

    return true;
}

} //end of anonymous namespace

void budget::load_pages(httplib::Server& server) {
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
    server.get("/overview/aggregate/all/", &overview_aggregate_all_page);
    server.get("/overview/savings/time/", &time_graph_savings_rate_page);

    server.get("/report/", &report_page);

    server.get("/accounts/", &accounts_page);
    server.get("/accounts/all/", &all_accounts_page);
    server.get("/accounts/add/", &add_accounts_page);
    server.post("/accounts/edit/", &edit_accounts_page);
    server.get("/accounts/archive/month/", &archive_accounts_month_page);
    server.get("/accounts/archive/year/", &archive_accounts_year_page);

    server.get(R"(/expenses/(\d+)/(\d+)/)", &expenses_page);
    server.get("/expenses/", &expenses_page);
    server.get("/expenses/search/", &search_expenses_page);

    server.get(R"(/expenses/breakdown/month/(\d+)/(\d+)/)", &month_breakdown_expenses_page);
    server.get("/expenses/breakdown/month/", &month_breakdown_expenses_page);

    server.get(R"(/expenses/breakdown/year/(\d+)/)", &year_breakdown_expenses_page);
    server.get("/expenses/breakdown/year/", &year_breakdown_expenses_page);

    server.get("/expenses/time/", &time_graph_expenses_page);
    server.get("/expenses/all/", &all_expenses_page);
    server.get("/expenses/add/", &add_expenses_page);
    server.post("/expenses/edit/", &edit_expenses_page);

    server.get(R"(/earnings/(\d+)/(\d+)/)", &earnings_page);
    server.get("/earnings/", &earnings_page);

    server.get("/earnings/time/", &time_graph_earnings_page);
    server.get("/income/time/", &time_graph_income_page);
    server.get("/earnings/all/", &all_earnings_page);
    server.get("/earnings/add/", &add_earnings_page);
    server.post("/earnings/edit/", &edit_earnings_page);

    server.get("/portfolio/status/", &portfolio_status_page);
    server.get("/portfolio/graph/", &portfolio_graph_page);
    server.get("/portfolio/currency/", &portfolio_currency_page);
    server.get("/portfolio/allocation/", &portfolio_allocation_page);
    server.get("/rebalance/", &rebalance_page);
    server.get("/assets/", &assets_page);
    server.get("/net_worth/status/", &net_worth_status_page);
    server.get("/net_worth/status/small/", &net_worth_small_status_page); // Not in the menu for now
    server.get("/net_worth/graph/", &net_worth_graph_page);
    server.get("/net_worth/currency/", &net_worth_currency_page);
    server.get("/net_worth/allocation/", &net_worth_allocation_page);
    server.get("/assets/add/", &add_assets_page);
    server.post("/assets/edit/", &edit_assets_page);

    server.get("/asset_values/list/", &list_asset_values_page);
    server.get("/asset_values/add/", &add_asset_values_page);
    server.get("/asset_values/batch/full/", &full_batch_asset_values_page);
    server.get("/asset_values/batch/current/", &current_batch_asset_values_page);
    server.post("/asset_values/edit/", &edit_asset_values_page);

    server.get("/objectives/list/", &list_objectives_page);
    server.get("/objectives/status/", &status_objectives_page);
    server.get("/objectives/add/", &add_objectives_page);
    server.post("/objectives/edit/", &edit_objectives_page);

    server.get("/wishes/list/", &wishes_list_page);
    server.get("/wishes/status/", &wishes_status_page);
    server.get("/wishes/estimate/", &wishes_estimate_page);
    server.get("/wishes/add/", &add_wishes_page);
    server.post("/wishes/edit/", &edit_wishes_page);

    server.get("/retirement/status/", &retirement_status_page);
    server.get("/retirement/configure/", &retirement_configure_page);
    server.get("/retirement/fi/", &retirement_fi_ratio_over_time);

    server.get("/recurrings/list/", &recurrings_list_page);
    server.get("/recurrings/add/", &add_recurrings_page);
    server.post("/recurrings/edit/", &edit_recurrings_page);

    server.get("/debts/list/", &budget::list_debts_page);
    server.get("/debts/all/", &budget::all_debts_page);
    server.get("/debts/add/", &budget::add_debts_page);
    server.post("/debts/edit/", &budget::edit_debts_page);

    server.get("/fortunes/graph/", &graph_fortunes_page);
    server.get("/fortunes/status/", &status_fortunes_page);
    server.get("/fortunes/list/", &list_fortunes_page);
    server.get("/fortunes/add/", &add_fortunes_page);
    server.post("/fortunes/edit/", &edit_fortunes_page);

    // Handle error

    server.set_error_handler([](const auto& req, auto& res) {
        std::stringstream content_stream;
        content_stream.imbue(std::locale("C"));

        if (res.status == 401 || res.status == 403) {
            content_stream << header("", false);
        } else {
            content_stream << header("", true);
        }

        content_stream << "<p>Error Status: <span class='text-danger'>";
        content_stream << res.status;
        content_stream << "</span></p>";

        content_stream << "<p>On Page: <span class='text-success'>";
        content_stream << req.path;
        content_stream << "</span></p>";

        content_stream << footer();

        res.set_content(content_stream.str(), "text/html");
    });
}

bool budget::page_start(const httplib::Request& req, httplib::Response& res, std::stringstream& content_stream, const std::string& title) {
    content_stream.imbue(std::locale("C"));

    if (is_secure()) {
        if (req.has_header("Authorization")) {
            auto authorization = req.get_header_value("Authorization");

            if (authorization.substr(0, 6) != "Basic ") {
                res.status = 401;
                res.set_header("WWW-Authenticate", "Basic realm=\"budgetwarrior\"");

                return false;
            }

            auto sub_authorization = authorization.substr(6, authorization.size());
            auto decoded           = base64_decode(sub_authorization);

            if (decoded.find(':') == std::string::npos) {
                res.status = 401;
                res.set_header("WWW-Authenticate", "Basic realm=\"budgetwarrior\"");

                return false;
            }

            auto username = decoded.substr(0, decoded.find(':'));
            auto password = decoded.substr(decoded.find(':') + 1, decoded.size());

            if (username != get_web_user()) {
                res.status = 401;
                res.set_header("WWW-Authenticate", "Basic realm=\"budgetwarrior\"");

                return false;
            }

            if (password != get_web_password()) {
                res.status = 401;
                res.set_header("WWW-Authenticate", "Basic realm=\"budgetwarrior\"");

                return false;
            }
        } else {
            res.status = 401;
            res.set_header("WWW-Authenticate", "Basic realm=\"budgetwarrior\"");

            return false;
        }
    }

    content_stream << header(title);

    budget::html_writer w(content_stream);
    display_message(w, req);

    return true;
}

void budget::page_end(budget::html_writer & w, const httplib::Request& req, httplib::Response& res) {
    w << "</main>";
    w.load_deferred_scripts();
    w << "</body></html>";

    auto result = w.os.str();

    filter_html(result, req);

    res.set_content(result, "text/html");
}

void budget::make_tables_sortable(budget::html_writer& w){
    w.defer_script(R"=====(
        $(".table").DataTable({
         "columnDefs": [ {
          "targets": 'not-sortable',
          "orderable": false,
         }]
        });
    )=====");

    w.use_module("datatables");
}

bool budget::page_get_start(const httplib::Request& req, httplib::Response& res,
                    std::stringstream& content_stream, const std::string& title, std::vector<const char*> parameters) {
    if (!page_start(req, res, content_stream, title)) {
        return false;
    }

    if (!validate_parameters(content_stream, req, parameters)){
        return false;
    }

    return true;
}

void budget::display_error_message(budget::writer& w, const std::string& message) {
    w << R"=====(<div class="alert alert-danger" role="alert">)=====";
    w << message;
    w << R"=====(</div>)=====";
}

void budget::form_begin(budget::writer& w, const std::string& action, const std::string& back_page) {
    w << R"=====(<form method="POST" action=")=====";
    w << action;
    w << R"=====(">)=====";
    w << R"=====(<input type="hidden" name="server" value="yes">)=====";
    w << R"=====(<input type="hidden" name="back_page" value=")=====";
    w << back_page;
    w << R"=====(">)=====";
}

void budget::page_form_begin(budget::writer& w, const std::string& action) {
    w << R"=====(<form method="GET" action=")=====";
    w << action;
    w << R"=====(">)=====";
}

void budget::form_begin_edit(budget::writer& w, const std::string& action, const std::string& back_page, const std::string& input_id) {
    form_begin(w, action, back_page);

    w << R"=====(<input type="hidden" name="input_id" value=")=====";
    w << input_id;
    w << R"=====(">)=====";
}

void budget::form_end(budget::writer& w, const std::string& button) {
    if (button.empty()) {
        w << R"=====(<button type="submit" class="btn btn-primary">Submit</button>)=====";
    } else {
        w << R"=====(<button type="submit" class="btn btn-primary">)=====";
        w << button;
        w << R"=====(</button>)=====";
    }

    w << "</form>";
}

void budget::add_text_picker(budget::writer& w, const std::string& title, const std::string& name, const std::string& default_value) {
    w << R"=====(<div class="form-group">)=====";

    w << "<label for=\"" << name << "\">" << title << "</label>";

    w << "<input required type=\"text\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";

    if (default_value.empty()) {
        w << " placeholder=\"Enter " << title << "\"";
    } else {
        w << " value=\"" << default_value << "\" ";
    }

    w << R"=====(
            >
         </div>
    )=====";
}


void budget::add_name_picker(budget::writer& w, const std::string& default_value) {
    add_text_picker(w, "Name", "input_name", default_value);
}

void budget::add_title_picker(budget::writer& w, const std::string& default_value) {
    add_text_picker(w, "Title", "input_title", default_value);
}

void budget::add_amount_picker(budget::writer& w, const std::string& default_value) {
    add_money_picker(w, "amount", "input_amount", default_value);
}

void budget::add_paid_amount_picker(budget::writer& w, const std::string& default_value) {
    add_money_picker(w, "paid amount", "input_paid_amount", default_value);
}

void budget::add_yes_no_picker(budget::writer& w, const std::string& title, const std::string& name, bool default_value) {
    w << R"=====(<div class="form-group">)=====";

    w << "<label for=\"" << name << "\">" << title << "</label>";

    if (default_value) {
        w << R"=====(<label class="radio-inline"><input type="radio" name=")=====";
        w << name;
        w << R"=====(" value="yes" checked>Yes</label>)=====";
    } else {
        w << R"=====(<label class="radio-inline"><input type="radio" name=")=====";
        w << name;
        w << R"=====(" value="yes">Yes</label>)=====";
    }

    if (!default_value) {
        w << R"=====(<label class="radio-inline"><input type="radio" name=")=====";
        w << name;
        w << R"=====(" value="no" checked>No</label>)=====";
    } else {
        w << R"=====(<label class="radio-inline"><input type="radio" name=")=====";
        w << name;
        w << R"=====(" value="no">No</label>)=====";
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void budget::add_paid_picker(budget::writer& w, bool paid) {
    add_yes_no_picker(w, "Paid", "input_paid", paid);
}

void budget::add_date_picker(budget::writer& w, const std::string& default_value, bool one_line) {
    if (one_line) {
        w << R"=====(<div class="form-group row">)=====";

        w << "<label class=\"col-sm-4 col-form-label\" for=\"input_date\">Date</label>";

        w << R"=====(<div class="col-sm-4">)=====";
    } else {
        w << R"=====(<div class="form-group">)=====";

        w << "<label for=\"input_date\">Date</label>";
    }

    auto today = budget::local_day();

    w << R"=====(<input required type="date" class="form-control" id="input_date" name="input_date" value=")=====";

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

    w << "\">";

    if (one_line) {
        w << "</div>";
        w << "</div>";
    } else {
        w << "</div>";
    }
}

std::stringstream budget::start_chart_base(budget::html_writer& w, const std::string& chart_type, const std::string& id, std::string style) {
    w.use_module("highcharts");

    w << R"=====(<div id=")=====";
    w << id;

    if (style.empty()) {
        w << R"=====(" class="default-graph-style"></div>)=====" << end_of_line;
    } else {
        w << R"=====(" style="margin: 0 auto; )=====";
        w << style;
        w << R"=====("></div>)=====" << end_of_line;
    }

    std::stringstream ss;
    ss.imbue(std::locale("C"));

    ss << R"=====(Highcharts.chart(')=====";
    ss << id;
    ss << R"=====(', {)=====";

    ss << R"=====(chart: {type: ')=====";
    ss << chart_type;
    ss << R"=====('},)=====";

    ss << R"=====(credits: { enabled: false },)=====";
    ss << R"=====(exporting: { enabled: false },)=====";

    return ss;
}

std::stringstream budget::start_chart(budget::html_writer& w, const std::string& title, const std::string& chart_type,
                                      const std::string& id, std::string style) {
    auto ss = start_chart_base(w, chart_type, id, style);

    ss << R"=====(title: {text: ')=====";
    ss << title;
    ss << R"=====('},)=====";

    return ss;
}

std::stringstream budget::start_time_chart(budget::html_writer& w, const std::string& title, const std::string& chart_type, 
                                           const std::string& id, std::string style) {
    // Note: Not nice but we are simply injecting zoomType here
    auto ss = start_chart_base(w, chart_type + "', zoomType: 'x", id, style);

    ss << R"=====(title: {text: ')=====";
    ss << title;
    ss << R"=====('},)=====";

    ss << R"=====(rangeSelector: {enabled: true}, )=====";

    return ss;
}

void budget::end_chart(budget::html_writer& w, std::stringstream& ss) {
    ss << R"=====(});)=====";

    w.defer_script(ss.str());
}

void budget::add_account_picker(budget::writer& w, budget::date day, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_account">Account</label>
                <select class="form-control" id="input_account" name="input_account">
    )=====";

    for (auto& account : all_accounts(day.year(), day.month())) {
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

void budget::add_asset_picker(budget::writer& w, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_asset">Asset</label>
                <select class="form-control" id="input_asset" name="input_asset">
    )=====";

    for (auto& asset : all_user_assets()) {
        if (budget::to_string(asset.id) == default_value) {
            w << "<option selected value=\"" << asset.id << "\">" << asset.name << "</option>";
        } else {
            w << "<option value=\"" << asset.id << "\">" << asset.name << "</option>";
        }
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void budget::add_money_picker(budget::writer& w, const std::string& title, const std::string& name, const std::string& default_value, bool one_line, const std::string& currency) {
    if(!currency.empty()){
        cpp_assert(one_line, "add_money_picker currency only works with one_line");
    }

    if (one_line) {
        w << R"=====(<div class="form-group row">)=====";

        w << "<label class=\"col-sm-4 col-form-label\" for=\"" << name << "\">" << title << "</label>";

        w << R"=====(<div class="col-sm-4">)=====";
    } else {
        w << R"=====(<div class="form-group">)=====";

        w << "<label for=\"" << name << "\">" << title << "</label>";
    }

    w << "<input required type=\"number\" step=\"0.01\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";

    if (default_value.empty()) {
        w << " placeholder=\"Enter " << title << "\" ";
    } else {
        w << " value=\"" << default_value << "\" ";
    }

    w << ">";

    if (one_line) {
        w << "</div>";

        if (!currency.empty()) {
            w << "<label class=\"col-sm-2 col-form-label\">" << currency << "</label>";
        }

        w << "</div>";
    } else {
        w << "</div>";
    }
}
