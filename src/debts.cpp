//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "debts.hpp"
#include "guid.hpp"

using namespace budget;

int budget::handle_debts(const std::vector<std::string>& args){
    if(args.size() == 1){
        std::cout << "List of debts" << std::endl;

        //TODO Implement display
    } else {
        auto& subcommand = args[1];

        if(subcommand == "add"){
            std::cout << "Add a new debt" << std::endl;

            std::string guid = generate_guid();

            std::cout << guid << std::endl;

            //TODO Implement creation of debts
        } else {
            std::cout << "Invalid subcommand \"" << subcommand << "\"" << std::endl;

            return 1;
        }
    }

    return 0;
}
