//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>

#include "config.hpp"
#include "args.hpp"

#include "debts.hpp"
#include "accounts.hpp"
#include "expenses.hpp"
#include "budget_exception.hpp"

using namespace budget;

int main(int argc, const char* argv[]) {
    //Verify that the data folder exists
    if(!verify_folder()){
        return 0;
    }

    if(argc == 1){
        //TODO In the future, display the default module
        std::cout << "A command is necessary" << std::endl;

        return 1;
    }

    auto args = parse_args(argc, argv);

    auto& command = args[0];

    try {
        if(command == "help"){
            std::cout << "Usage: budget command [options]" << std::endl;

            //TODO Display complete help
        } else if(command == "debt"){
            return handle_debts(args);
        } else if(command == "account"){
            return handle_accounts(args);
        } else if(command == "expense"){
            return handle_expenses(args);
        } else {
            std::cout << "Unhandled command \"" << command << "\"" << std::endl;

            return 1;
        }
    } catch (const budget_exception& exception){
        std::cout << exception.message() << std::endl;

        return 1;
    }

    return 0;
}
