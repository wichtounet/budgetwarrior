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

template<typename Values>
std::size_t gc(Values& values){
    std::sort(values.begin(), values.end(), 
        [](const typename Values::value_type& a, const typename Values::value_type& b){ return a.id < b.id; });

    std::size_t next_id = 0;

    for(auto& expense : values){
        expense.id = ++next_id;
    }

    return ++next_id;
}

void gc_expenses(){
    auto next_id = gc(all_expenses());
    set_expenses_next_id(next_id);
    set_expenses_changed();
}

void gc_earnings(){
    auto next_id = gc(all_earnings());
    set_earnings_next_id(next_id);
    set_earnings_changed();
}

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

        gc_expenses();
        gc_earnings();

        std::cout << "...done" << std::endl;
    }
}
