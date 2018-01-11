//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "version.hpp"
#include "budget_exception.hpp"

std::string budget::get_version(){
    return "budgetwarrior 1.0";
}

std::string budget::get_version_short(){
    return "1.0";
}

void budget::version_module::handle(const std::vector<std::string>& args){
    if(args.size() > 1){
        throw budget_exception("Too many arguments to version");
    }

    std::cout << get_version() << std::endl;
    std::cout << "Copyright 2013-2018 Baptiste Wicht" << std::endl;
}
