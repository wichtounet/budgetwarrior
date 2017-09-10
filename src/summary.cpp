//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>
#include <unordered_map>
#include <unordered_set>

#include "cpp_utils/assert.hpp"

#include "summary.hpp"
#include "console.hpp"
#include "accounts.hpp"
#include "compute.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "config.hpp"

using namespace budget;

namespace {

void month_overview(budget::month m, budget::year y){
    // TODO
}

void month_overview(budget::month m){
    month_overview(m, local_day().year());
}

void month_overview(){
    month_overview(local_day().month(), local_day().year());
}

} // end of anonymous namespace

constexpr const std::array<std::pair<const char*, const char*>, 1> budget::module_traits<budget::summary_module>::aliases;

void budget::summary_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::summary_module::handle(std::vector<std::string>& args){
    if(all_accounts().empty()){
        throw budget_exception("No accounts defined, you should start by defining some of them");
    }

    if(args.empty() || args.size() == 1){
        month_overview();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "month"){
            if(args.size() == 2){
                month_overview();
            } else if(args.size() == 3){
                month_overview(budget::month(to_number<unsigned short>(args[2])));
            } else if(args.size() == 4){
                month_overview(
                    budget::month(to_number<unsigned short>(args[2])),
                    budget::year(to_number<unsigned short>(args[3])));
            } else {
                throw budget_exception("Too many arguments to overview month");
            }
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

