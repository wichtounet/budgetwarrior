//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <unordered_map>
#include <unordered_set>

#include "predict.hpp"
#include "overview.hpp"
#include "console.hpp"
#include "accounts.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"
#include "config.hpp"
#include "assert.hpp"

using namespace budget;

namespace {

void predict_overview(){

}

} // end of anonymous namespace

void budget::predict_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::predict_module::handle(std::vector<std::string>& args){
    if(all_accounts().empty()){
        throw budget_exception("No accounts defined, you should start by defining some of them");
    }

    if(args.empty() || args.size() == 1){
        predict_overview();
    } else {
        throw budget_exception("Too many arguments");
    }
}
