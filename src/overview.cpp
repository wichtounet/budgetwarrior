//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "overview.hpp"
#include "accounts.hpp"
#include "expenses.hpp"
#include "budget_exception.hpp"

using namespace budget;

void budget::handle_overview(const std::vector<std::string>& args){
    load_accounts();
    load_expenses();

    if(args.size() == 1){
        month_overview();
    } else {
        auto& subcommand = args[1];

        if(subcommand == "month"){
            month_overview();
        } else if(subcommand == "year"){
            year_overview();
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}

void budget::month_overview(){
    std::cout << "Overview of the month" << std::endl;

    //TODO
}

void budget::year_overview(){
    std::cout << "Overview of the year" << std::endl;

    //TODO
}
