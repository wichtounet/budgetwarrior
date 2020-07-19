//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "pages/index_pages.hpp"
#include "pages/earnings_pages.hpp"
#include "pages/expenses_pages.hpp"
#include "pages/objectives_pages.hpp"
#include "pages/net_worth_pages.hpp"
#include "writer.hpp"
#include "http.hpp"
#include "config.hpp"
#include "assets.hpp"
#include "earnings.hpp"
#include "expenses.hpp"
#include "accounts.hpp"
#include "incomes.hpp"

using namespace budget;

namespace {

budget::money monthly_income(budget::month month, budget::year year) {
    std::map<size_t, budget::money> account_sum;

    for (auto& earning : all_earnings()) {
        if (earning.date.year() == year && earning.date.month() == month) {
            account_sum[earning.account] += earning.amount;
        }
    }

    budget::money total = get_base_income();

    for (auto& [id, sum] : account_sum) {
        total += sum;
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

    for (auto& [id, sum] : account_sum) {
        total += sum;
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

} // namespace

void budget::index_page(const httplib::Request& req, httplib::Response& res) {
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
