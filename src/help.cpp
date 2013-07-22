//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "help.hpp"

void budget::help_module::handle(const std::vector<std::string>& args){
    std::cout << "Usage: budget                                                     Display the overview of the current month" << std::endl << std::endl;

    std::cout << "       budget account                                             Show all the accounts" << std::endl;
    std::cout << "       budget account show                                        Show the current accounts" << std::endl;
    std::cout << "       budget account all                                         Show all the accounts" << std::endl;
    std::cout << "       budget account add [name] [amount]                         Add a new account" << std::endl;
    std::cout << "       budget account delete [id]                                 Delete the given account" << std::endl;
    std::cout << "       budget account edit [id]                                   Edit the given account" << std::endl << std::endl;
    std::cout << "       budget account archive                                     Archive the current accounts to start new accounts" << std::endl << std::endl;

    std::cout << "       budget expense                                             Display the expenses of the current month" << std::endl;
    std::cout << "       budget expense show (month) (year)                         Display the expenses of the specified month of the specified year" << std::endl;
    std::cout << "       budget expense all                                         Display all the expenses" << std::endl;
    std::cout << "       budget expense add [account] [amount] [name]               Add a new expense on the current date" << std::endl;
    std::cout << "       budget expense addd [date] [account] [amount] [name]       Add a new expense on the given date" << std::endl;
    std::cout << "       budget expense delete [id]                                 Remove completely the expense with the given id" << std::endl;
    std::cout << "       budget expense edit [id]                                   Modify the expense with the given id" << std::endl << std::endl;

    std::cout << "       budget earning                                             Display the earnings of the current month" << std::endl;
    std::cout << "       budget earning show (month) (year)                         Display the earnings of the specified month of the specified year" << std::endl;
    std::cout << "       budget earning all                                         Display all the earnings" << std::endl;
    std::cout << "       budget earning add [account] [amount] [name]               Add a new earning on the current date" << std::endl;
    std::cout << "       budget earning addd [date] [account] [amount] [name]       Add a new earning on the given date" << std::endl;
    std::cout << "       budget earning delete [id]                                 Remove completely the earning with the given id" << std::endl;
    std::cout << "       budget earning edit [id]                                   Modify the earning with the given id" << std::endl << std::endl;

    std::cout << "       budget debt                                                Display the unpaid debts" << std::endl;
    std::cout << "       budget debt list                                           Display the unpaid debts" << std::endl;
    std::cout << "       budget debt all                                            Display the unpaid debts" << std::endl;
    std::cout << "       budget debt all                                            Display all the debts" << std::endl;
    std::cout << "       budget debt paid [id]                                      Mark the given debt as paid" << std::endl;
    std::cout << "       budget debt delete [id]                                    Delete the given debt" << std::endl;
    std::cout << "       budget debt edit [id]                                      Edit the given debt" << std::endl << std::endl;

    std::cout << "       budget overview                                            Display the overvew of the current month" << std::endl;
    std::cout << "       budget overview month (month) (year)                       Display the overvew of the specified month of the current year" << std::endl;
    std::cout << "       budget overview year (year)                                Display the overvew of the specified year" << std::endl << std::endl;
}
