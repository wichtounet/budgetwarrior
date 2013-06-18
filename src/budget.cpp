//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>

int main(int argc, const char* argv[]) {
    if(argc == 1){
        std::cout << "A command is necessary" << std::endl;

        return 1;
    }

    std::string command(argv[1]);

    if(command == "help"){
        std::cout << "Usage: budget command [options]" << std::endl;

        //TODO Display complete help
    } else if(command == "debt"){
        if(argc == 2){
            std::cout << "List of debts" << std::endl;

            //TODO Implement display
        } else if(argc > 2){
            std::string subcommand(argv[2]);

            if(subcommand == "add"){
                std::cout << "Add a new debt" << std::endl;

                //TODO Implement creation of debts
            } else {
                std::cout << "Invalid subcommand \"" << subcommand << "\"" << std::endl;

                return 1;
            }

        }
    } else {
        std::cout << "Invalid command \"" << command << "\"" << std::endl;

        return 1;
    }

    return 0;
}
