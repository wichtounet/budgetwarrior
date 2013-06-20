//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "debts.hpp"
#include "guid.hpp"
#include "money.hpp"

using namespace budget;

int budget::handle_debts(const std::vector<std::string>& args){
    if(args.size() == 1){
        std::cout << "List of debts" << std::endl;

        //TODO Implement display
    } else {
        auto& subcommand = args[1];

        if(subcommand == "add"){
            if(args.size() < 5){
                std::cout << "Not enough args for debt add" << std::endl;

                return 1;
            }

            std::string guid = generate_guid();
            std::string direction = args[2];
            std::string name = args[3];
            std::string amount_string = args[4];
            std::string title = "";

            if(args.size() > 5){
                for(std::size_t i = 5; i < args.size(); ++i){
                    title += args[i];
                }
            }

            if(direction != "to" && direction != "from"){
                std::cout << "Invalid direction, only \"to\" and \"from\" are valid" << std::endl;

                return 1;
            }

            auto amount = parse_money(amount_string);

            if(amount.dollars < 0 || amount.cents < 0){
                std::cout << "Amount of the debt cannot be negative" << std::endl;

                return 1;
            }

            //TODO Implement creation of debts
        } else {
            std::cout << "Invalid subcommand \"" << subcommand << "\"" << std::endl;

            return 1;
        }
    }

    return 0;
}
