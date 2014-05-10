//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "versioning.hpp"
#include "budget_exception.hpp"

void budget::versioning_module::handle(const std::vector<std::string>& args){
    if(args.size() == 1){
        std::cout << "Missing subcommand" << std::endl;
    } else {
        auto& subcommand = args[1];

        if(subcommand == "save"){
            //TODO
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}
