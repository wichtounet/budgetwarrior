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
#include "budget_exception.hpp"

#include "debts.hpp"
#include "accounts.hpp"
#include "expenses.hpp"
#include "overview.hpp"

using namespace budget;

int main(int argc, const char* argv[]) {
    //Verify that the data folder exists
    if(!verify_folder()){
        return 0;
    }

    if(argc == 1){
        month_overview();
    } else {
        auto args = parse_args(argc, argv);

        auto& command = args[0];

        try {
            if(command == L"help"){
                std::wcout << L"Usage: budget command [options]" << std::endl;

                //TODO Display complete help
            } else if(command == L"debt"){
                handle_debts(args);
            } else if(command == L"account"){
                handle_accounts(args);
            } else if(command == L"expense"){
                handle_expenses(args);
            } else if(command == L"overview"){
                handle_overview(args);
            } else {
                std::wcout << L"Unhandled command \"" << command << L"\"" << std::endl;

                return 1;
            }
        } catch (const budget_exception& exception){
            std::wcout << exception.message() << std::endl;

            return 1;
        }
    }

    return 0;
}
