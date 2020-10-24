//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>
#include <numeric>

#include "cpp_utils/assert.hpp"

#include "data_cache.hpp"
#include "config.hpp"
#include "overview.hpp"
#include "summary.hpp"
#include "version.hpp"
#include "writer.hpp"
#include "currency.hpp"

// Include all the pages
#include "pages/assets_pages.hpp"
#include "pages/asset_values_pages.hpp"
#include "pages/asset_shares_pages.hpp"
#include "pages/asset_classes_pages.hpp"
#include "pages/liabilities_pages.hpp"
#include "pages/fortunes_pages.hpp"
#include "pages/wishes_pages.hpp"
#include "pages/index_pages.hpp"
#include "pages/report_pages.hpp"
#include "pages/debts_pages.hpp"
#include "pages/accounts_pages.hpp"
#include "pages/incomes_pages.hpp"
#include "pages/expenses_pages.hpp"
#include "pages/earnings_pages.hpp"
#include "pages/retirement_pages.hpp"
#include "pages/recurrings_pages.hpp"
#include "pages/objectives_pages.hpp"
#include "pages/overview_pages.hpp"
#include "pages/net_worth_pages.hpp"
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

        bool side_hustle = !config_value("side_category", "").empty() && !config_value("side_prefix", "").empty();

        // Overview

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown01" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Overview</a>
                <div class="dropdown-menu" aria-labelledby="dropdown01">
                  <a class="dropdown-item" href="/overview/">Overview Month</a>
                  <a class="dropdown-item" href="/overview/year/">Overview Year</a>
                  <a class="dropdown-item" href="/overview/aggregate/year/">Aggregate Year</a>
                  <a class="dropdown-item" href="/overview/aggregate/year_month/">Aggregate Year per month</a>
                  <a class="dropdown-item" href="/overview/aggregate/month/">Aggregate Month</a>
                  <a class="dropdown-item" href="/overview/aggregate/all/">Aggregate All</a>
        )=====";

        if (side_hustle) {
            stream << R"=====(
                  <a class="dropdown-item" href="/side_hustle/overview/">Side Hustle Overview Month</a>
            )=====";
        }

        stream << R"=====(
                  <a class="dropdown-item" href="/report/">Report</a>
                  <a class="dropdown-item" href="/overview/savings/time/">Savings rate over time</a>
                  <a class="dropdown-item" href="/overview/tax/time/">Tax rate over time</a>
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
                  <a class="dropdown-item" href="/assets/graph/">Asset Graph</a>
                  <a class="dropdown-item" href="/asset_values/list/">Asset Values</a>
                  <a class="dropdown-item" href="/asset_values/batch/full/">Full Batch Update</a>
                  <a class="dropdown-item" href="/asset_values/batch/current/">Current Batch Update</a>
                  <a class="dropdown-item" href="/asset_values/add/">Set One Asset Value</a>
                  <a class="dropdown-item" href="/asset_shares/list/">Asset Shares</a>
                  <a class="dropdown-item" href="/asset_shares/add/">Add Asset Share</a>
                  <div class="dropdown-divider"></div>
                  <a class="dropdown-item" href="/liabilities/">Liabilities</a>
                  <a class="dropdown-item" href="/liabilities/add/">Add Liability</a>
                  <a class="dropdown-item" href="/asset_values/add/liability/">Set One Liability Value</a>
                  <div class="dropdown-divider"></div>
                  <a class="dropdown-item" href="/asset_classes/list/">Asset Classes</a>
                  <a class="dropdown-item" href="/asset_classes/add/">Add Asset Class</a>
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
                  <a class="dropdown-item" href="/earnings/search/">Search</a>
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
                  <div class="dropdown-divider"></div>
                  <a class="dropdown-item" href="/incomes/">Incomes</a>
                  <a class="dropdown-item" href="/incomes/set/">Set Income</a>
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
        )=====";

        if (budget::is_debts_disabled()) {
            stream << R"=====(
                      <div class="dropdown-divider"></div>
                      <a class="dropdown-item" href="/debts/add/">Add Debt</a>
                      <a class="dropdown-item" href="/debts/list/">List Debts</a>
                      <a class="dropdown-item" href="/debts/all/">All Debts</a>
                    </div>
                  </li>
            )=====";
        }

        stream << R"=====(
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
    if (req.has_param("input_name")) {
        replace_all(html, "__budget_this_page__", html_base64_encode(req.path + "?input_name=" + req.get_param_value("input_name")));
    } else {
        replace_all(html, "__budget_this_page__", html_base64_encode(req.path));
    }

    replace_all(html, "__currency__", get_default_currency());
}

//Note: This must be synchronized with page_end
std::string footer() {
    return "</main></body></html>";
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
    server.Get("/", &index_page);

    server.Get("/overview/year/", &overview_year_page);
    server.Get(R"(/overview/year/(\d+)/)", &overview_year_page);
    server.Get("/overview/", &overview_page);
    server.Get(R"(/overview/(\d+)/(\d+)/)", &overview_page);
    server.Get("/overview/aggregate/year/", &overview_aggregate_year_page);
    server.Get(R"(/overview/aggregate/year/(\d+)/)", &overview_aggregate_year_page);
    server.Get("/overview/aggregate/year_month/", &overview_aggregate_year_month_page);
    server.Get(R"(/overview/aggregate/year_month/(\d+)/)", &overview_aggregate_year_month_page);
    server.Get("/overview/aggregate/month/", &overview_aggregate_month_page);
    server.Get(R"(/overview/aggregate/month/(\d+)/(\d+)/)", &overview_aggregate_month_page);
    server.Get("/overview/aggregate/all/", &overview_aggregate_all_page);
    server.Get("/overview/savings/time/", &time_graph_savings_rate_page);
    server.Get("/overview/tax/time/", &time_graph_tax_rate_page);

    bool side_hustle = !config_value("side_category", "").empty() && !config_value("side_prefix", "").empty();
    if (side_hustle) {
        server.Get("/side_hustle/overview/", &side_overview_page);
        server.Get(R"(/side_hustle/overview/(\d+)/(\d+)/)", &side_overview_page);
    }

    server.Get("/report/", &report_page);

    server.Get("/accounts/", &accounts_page);
    server.Get("/accounts/all/", &all_accounts_page);
    server.Get("/accounts/add/", &add_accounts_page);
    server.Get("/accounts/edit/", &edit_accounts_page);
    server.Get("/accounts/archive/month/", &archive_accounts_month_page);
    server.Get("/accounts/archive/year/", &archive_accounts_year_page);

    server.Get("/incomes/", &incomes_page);
    server.Get("/incomes/set/", &set_incomes_page);

    server.Get(R"(/expenses/(\d+)/(\d+)/)", &expenses_page);
    server.Get("/expenses/", &expenses_page);
    server.Get("/expenses/search/", &search_expenses_page);

    server.Get(R"(/expenses/breakdown/month/(\d+)/(\d+)/)", &month_breakdown_expenses_page);
    server.Get("/expenses/breakdown/month/", &month_breakdown_expenses_page);

    server.Get(R"(/expenses/breakdown/year/(\d+)/)", &year_breakdown_expenses_page);
    server.Get("/expenses/breakdown/year/", &year_breakdown_expenses_page);

    server.Get("/expenses/time/", &time_graph_expenses_page);
    server.Get("/expenses/all/", &all_expenses_page);
    server.Get("/expenses/add/", &add_expenses_page);
    server.Get("/expenses/edit/", &edit_expenses_page);

    server.Get(R"(/earnings/(\d+)/(\d+)/)", &earnings_page);
    server.Get("/earnings/", &earnings_page);
    server.Get("/earnings/search/", &search_earnings_page);

    server.Get("/earnings/time/", &time_graph_earnings_page);
    server.Get("/income/time/", &time_graph_income_page);
    server.Get("/earnings/all/", &all_earnings_page);
    server.Get("/earnings/add/", &add_earnings_page);
    server.Get("/earnings/edit/", &edit_earnings_page);

    server.Get("/portfolio/status/", &portfolio_status_page);
    server.Get("/portfolio/graph/", &portfolio_graph_page);
    server.Get("/portfolio/currency/", &portfolio_currency_page);
    server.Get("/portfolio/allocation/", &portfolio_allocation_page);
    server.Get("/rebalance/", &rebalance_page);
    server.Get("/rebalance/nocash/", &rebalance_nocash_page);
    server.Get("/assets/", &assets_page);
    server.Get("/net_worth/status/", &net_worth_status_page);
    server.Get("/net_worth/status/small/", &net_worth_small_status_page); // Not in the menu for now
    server.Get("/net_worth/graph/", &net_worth_graph_page);
    server.Get("/net_worth/currency/", &net_worth_currency_page);
    server.Get("/net_worth/allocation/", &net_worth_allocation_page);
    server.Get("/assets/add/", &add_assets_page);
    server.Get("/assets/edit/", &edit_assets_page);
    server.Get(R"(/assets/graph/(\d+)/)", &asset_graph_page);
    server.Get("/assets/graph/", &asset_graph_page);

    server.Get("/asset_values/list/", &list_asset_values_page);
    server.Get("/asset_values/add/", &add_asset_values_page);
    server.Get("/asset_values/add/liability/", &add_asset_values_liability_page);
    server.Get("/asset_values/batch/full/", &full_batch_asset_values_page);
    server.Get("/asset_values/batch/current/", &current_batch_asset_values_page);
    server.Get("/asset_values/edit/", &edit_asset_values_page);

    server.Get("/asset_shares/list/", &list_asset_shares_page);
    server.Get("/asset_shares/add/", &add_asset_shares_page);
    server.Get("/asset_shares/edit/", &edit_asset_shares_page);

    server.Get("/asset_classes/list/", &list_asset_classes_page);
    server.Get("/asset_classes/add/", &add_asset_classes_page);
    server.Get("/asset_classes/edit/", &edit_asset_classes_page);

    server.Get("/liabilities/", &list_liabilities_page);
    server.Get("/liabilities/list/", &list_liabilities_page);
    server.Get("/liabilities/add/", &add_liabilities_page);
    server.Get("/liabilities/edit/", &edit_liabilities_page);

    server.Get("/objectives/list/", &list_objectives_page);
    server.Get("/objectives/status/", &status_objectives_page);
    server.Get("/objectives/add/", &add_objectives_page);
    server.Get("/objectives/edit/", &edit_objectives_page);

    server.Get("/wishes/list/", &wishes_list_page);
    server.Get("/wishes/status/", &wishes_status_page);
    server.Get("/wishes/estimate/", &wishes_estimate_page);
    server.Get("/wishes/add/", &add_wishes_page);
    server.Get("/wishes/edit/", &edit_wishes_page);

    server.Get("/retirement/status/", &retirement_status_page);
    server.Get("/retirement/configure/", &retirement_configure_page);
    server.Get("/retirement/fi/", &retirement_fi_ratio_over_time);

    server.Get("/recurrings/list/", &recurrings_list_page);
    server.Get("/recurrings/add/", &add_recurrings_page);
    server.Get("/recurrings/edit/", &edit_recurrings_page);

    server.Get("/debts/list/", &budget::list_debts_page);
    server.Get("/debts/all/", &budget::all_debts_page);
    server.Get("/debts/add/", &budget::add_debts_page);
    server.Get("/debts/edit/", &budget::edit_debts_page);

    server.Get("/fortunes/graph/", &graph_fortunes_page);
    server.Get("/fortunes/status/", &status_fortunes_page);
    server.Get("/fortunes/list/", &list_fortunes_page);
    server.Get("/fortunes/add/", &add_fortunes_page);
    server.Get("/fortunes/edit/", &edit_fortunes_page);

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
    w << html_base64_encode(back_page);
    w << R"=====(">)=====";
}

void budget::page_form_begin(budget::writer& w, const std::string& action) {
    w << R"=====(<form method="GET" action=")=====";
    w << action;
    w << R"=====(">)=====";
}

void budget::form_begin_edit(budget::writer& w, const std::string& action, const std::string& back_page, const std::string& input_id) {
    form_begin(w, action, html_base64_decode(back_page));

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

void budget::add_text_picker(budget::writer& w, const std::string& title, const std::string& name, const std::string& default_value, bool required) {
    w << R"=====(<div class="form-group">)=====";

    w << "<label for=\"" << name << "\">" << title << "</label>";

    if (required) {
        w << "<input required type=\"text\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    } else {
        w << "<input type=\"text\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    }

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

void budget::add_average_12_serie(std::stringstream& ss,
                                 std::vector<budget::money> serie,
                                 std::vector<std::string> dates) {
    ss << "{ name: '12 months average',";
    ss << "data: [";

    std::array<budget::money, 12> average_12;

    for (size_t i = 0; i < serie.size(); ++i) {
        average_12[i % 12] = serie[i];

        auto average = std::accumulate(average_12.begin(), average_12.end(), budget::money());

        if (i < 12) {
            average = average / int(i + 1);
        } else {
            average = average / 12;
        }

        ss << "[" << dates[i] << "," << budget::to_flat_string(average) << "],";
    }

    ss << "]},";
}

void budget::add_average_5_serie(std::stringstream& ss,
                                 std::vector<budget::money> serie,
                                 std::vector<std::string> dates) {
    ss << "{ name: '5 year average',";
    ss << "data: [";

    std::array<budget::money, 5> average_5;

    for (size_t i = 0; i < serie.size(); ++i) {
        average_5[i % 5] = serie[i];

        auto average = std::accumulate(average_5.begin(), average_5.end(), budget::money());

        if (i < 5) {
            average = average / int(i + 1);
        } else {
            average = average / 5;
        }

        ss << "[" << dates[i] << "," << budget::to_flat_string(average) << "],";
    }

    ss << "]},";
}

void budget::add_account_picker(budget::writer& w, budget::date day, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_account">Account</label>
                <select class="form-control" id="input_account" name="input_account">
    )=====";

    data_cache cache;

    for (auto& account : all_accounts(cache, day.year(), day.month())) {
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

void budget::add_share_asset_picker(budget::writer& w, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_asset">Asset</label>
                <select class="form-control" id="input_asset" name="input_asset">
    )=====";

    data_cache cache;

    for (auto& asset : cache.user_assets()) {
        if (asset.share_based) {
            if (budget::to_string(asset.id) == default_value) {
                w << "<option selected value=\"" << asset.id << "\">" << asset.name << "</option>";
            } else {
                w << "<option value=\"" << asset.id << "\">" << asset.name << "</option>";
            }
        }
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void budget::add_value_asset_picker(budget::writer& w, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_asset">Asset</label>
                <select class="form-control" id="input_asset" name="input_asset">
    )=====";

    data_cache cache;

    for (auto& asset : cache.user_assets()) {
        if (!asset.share_based) {
            if (budget::to_string(asset.id) == default_value) {
                w << "<option selected value=\"" << asset.id << "\">" << asset.name << "</option>";
            } else {
                w << "<option value=\"" << asset.id << "\">" << asset.name << "</option>";
            }
        }
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void budget::add_liability_picker(budget::writer& w, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_asset">Liability</label>
                <select class="form-control" id="input_asset" name="input_asset">
    )=====";

    for (auto& liability : all_liabilities()) {
        if (budget::to_string(liability.id) == default_value) {
            w << "<option selected value=\"" << liability.id << "\">" << liability.name << "</option>";
        } else {
            w << "<option value=\"" << liability.id << "\">" << liability.name << "</option>";
        }
    }

    w << R"=====(
                </select>
            </div>
            <input type="hidden" name="input_liability" value="true" />
    )=====";
}

void budget::add_integer_picker(budget::writer& w, const std::string& title, const std::string& name, bool negative, const std::string& default_value) {
    w << R"=====(<div class="form-group">)=====";

    w << "<label for=\"" << name << "\">" << title << "</label>";

    if (negative) {
        w << "<input required type=\"number\" step=\"1\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    } else {
        w << "<input required type=\"number\" min=\"0\" step=\"1\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    }

    if (default_value.empty()) {
        w << " placeholder=\"Enter " << title << "\" ";
    } else {
        w << " value=\"" << default_value << "\" ";
    }

    w << ">";

    w << "</div>";
}

void budget::add_money_picker(budget::writer& w, const std::string& title, const std::string& name, const std::string& default_value, bool required,
                              bool one_line, const std::string& currency) {
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

    if (required) {
        w << "<input required type=\"number\" step=\"0.01\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    } else {
        w << "<input type=\"number\" step=\"0.01\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    }

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
