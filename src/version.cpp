//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "version.hpp"
#include "budget_exception.hpp"

void budget::version_module::handle(const std::vector<std::string>& args){
    if(args.size() > 1){
        throw budget_exception("Too many arguments to version");
    }

    std::cout << "budgetwarrior 0.4.3" << std::endl;
    std::cout << "Copyright 2013-2017 Baptiste Wicht" << std::endl;
}
