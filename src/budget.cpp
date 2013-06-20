//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>

#include "args.hpp"
#include "debts.hpp"

using namespace budget;

int main(int argc, const char* argv[]) {
    if(argc == 1){
        std::cout << "A command is necessary" << std::endl;

        return 1;
    }

    auto args = parse_args(argc, argv);

    auto& command = args[0];

    if(command == "help"){
        std::cout << "Usage: budget command [options]" << std::endl;

        //TODO Display complete help
    } else if(command == "debt"){
        return handle_debts(args);
    } else {
        std::cout << "Invalid command \"" << command << "\"" << std::endl;

        return 1;
    }

    return 0;
}
