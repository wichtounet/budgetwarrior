//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "report.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "budget_exception.hpp"

using namespace budget;

namespace {

void monthly_report(){

}

} //end of anonymous namespace

void budget::report_module::load(){
    load_expenses();
    load_earnings();
}

void budget::report_module::handle(const std::vector<std::string>& args){
    if(args.size() == 1){
        monthly_report();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "monthly"){
            monthly_report();
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}
