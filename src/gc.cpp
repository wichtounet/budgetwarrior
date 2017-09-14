//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "gc.hpp"
#include "accounts.hpp"
#include "debts.hpp"
#include "earnings.hpp"
#include "expenses.hpp"
#include "fortune.hpp"
#include "wishes.hpp"
#include "objectives.hpp"
#include "recurring.hpp"

using namespace budget;

namespace {

template<typename Values>
size_t gc(Values& values){
    std::sort(values.begin(), values.end(), 
        [](const typename Values::value_type& a, const typename Values::value_type& b){ return a.id < b.id; });

    size_t next_id = 0;

    for(auto& value : values){
        value.id = ++next_id;
    }

    return ++next_id;
}

template<typename Values>
void adapt(Values& values, size_t old, size_t id){
    for(auto& value : values){
        if(value.account == old){
            value.account = id;
        }
    }
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

void gc_debts(){
    auto next_id = gc(all_debts());
    set_debts_next_id(next_id);
    set_debts_changed();
}

void gc_fortunes(){
    auto next_id = gc(all_fortunes());
    set_fortunes_next_id(next_id);
    set_fortunes_changed();
}

void gc_wishes(){
    auto next_id = gc(all_wishes());
    set_wishes_next_id(next_id);
    set_wishes_changed();
}

void gc_objectives(){
    auto next_id = gc(all_objectives());
    set_objectives_next_id(next_id);
    set_objectives_changed();
}

void gc_recurrings(){
    auto next_id = gc(all_recurrings());
    set_recurrings_next_id(next_id);
    set_recurrings_changed();
}

void gc_accounts(){
    auto& accounts = all_accounts();

    size_t next_id = 1;

    std::sort(accounts.begin(), accounts.end(), [](const account& a, const account& b){ return a.id < b.id; });

    for(auto& account : accounts){
        if(account.id != next_id){
            auto old_account = account.id;
            account.id = next_id;

            adapt(all_expenses(), old_account, account.id);
            adapt(all_earnings(), old_account, account.id);

            //Note: No need to adapt recurrings since the account is stored with its name
        }

        ++next_id;
    }

    ++next_id;

    set_accounts_changed();
    set_accounts_next_id(next_id);
}

} //end of anonymous namespace

void budget::gc_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
    load_debts();
    load_fortunes();
    load_wishes();
    load_objectives();
    load_recurrings();
}

void budget::gc_module::unload(){
    save_expenses();
    save_earnings();
    save_accounts();
    save_debts();
    save_fortunes();
    save_wishes();
    save_objectives();
    save_recurrings();
}

void budget::gc_module::handle(const std::vector<std::string>& args){
    if(args.size() > 1){
        std::cout << "Too many parameters" << std::endl;
    } else {
        std::cout << "Make all IDs contiguous..." << std::endl;

        gc_expenses();
        gc_earnings();
        gc_debts();
        gc_fortunes();
        gc_wishes();
        gc_objectives();
        gc_recurrings();
        gc_accounts();

        std::cout << "...done" << std::endl;
    }
}
