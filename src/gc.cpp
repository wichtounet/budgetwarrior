//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "gc.hpp"
#include "accounts.hpp"
#include "expenses.hpp"
#include "earnings.hpp"

using namespace budget;

namespace {

} //end of anonymous namespace

void budget::gc_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::gc_module::unload(){
    save_expenses();
    save_earnings();
    save_accounts();
}

void budget::gc_module::handle(const std::vector<std::string>& args){
    if(args.size() > 1){
        std::cout << "Too many parameters" << std::endl;
    } else {
        std::cout << "Make all IDs contiguous..." << std::endl;

        std::cout << "...done" << std::endl;
    }
}
