//=======================================================================
// Copyright (c) 2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "help.hpp"

void budget::help_module::handle(const std::vector<std::string>&){
    std::cout << "Usage: budget                                                     Display the overview of the current month" << std::endl << std::endl;

    std::cout << "       budget account [show]                                      Show the current accounts" << std::endl;
    std::cout << "       budget account all                                         Show all the accounts" << std::endl;
    std::cout << "       budget account add                                         Add a new account" << std::endl;
    std::cout << "       budget account delete (id)                                 Delete the given account" << std::endl;
    std::cout << "       budget account edit (id)                                   Edit the given account" << std::endl;
    std::cout << "       budget account archive                                     Archive the current accounts to start new accounts" << std::endl << std::endl;

    std::cout << "       budget expense [show]                                      Display the expenses of the current month" << std::endl;
    std::cout << "       budget expense show (month) (year)                         Display the expenses of the specified month of the specified year" << std::endl;
    std::cout << "       budget expense all                                         Display all the expenses" << std::endl;
    std::cout << "       budget expense add                                         Add a new expense" << std::endl;
    std::cout << "       budget expense delete (id)                                 Remove completely the expense with the given id" << std::endl;
    std::cout << "       budget expense edit (id)                                   Modify the expense with the given id" << std::endl << std::endl;

    std::cout << "       budget earning [earnings]                                  Display the earnings of the current month" << std::endl;
    std::cout << "       budget earning show (month) (year)                         Display the earnings of the specified month of the specified year" << std::endl;
    std::cout << "       budget earning all                                         Display all the earnings" << std::endl;
    std::cout << "       budget earning add                                         Add a new earning" << std::endl;
    std::cout << "       budget earning delete (id)                                 Remove completely the earning with the given id" << std::endl;
    std::cout << "       budget earning edit (id)                                   Modify the earning with the given id" << std::endl << std::endl;

    std::cout << "       budget fortune [status]                                    Display the status of the fortune checks" << std::endl;
    std::cout << "       budget fortune list                                        Display the list of all the fortune checks" << std::endl;
    std::cout << "       budget fortune check                                       Create a new fortune check" << std::endl;
    std::cout << "       budget fortune edit (id)                                   Edit a fortune check" << std::endl;
    std::cout << "       budget fortune delete (id)                                 Delete a fortune check" << std::endl << std::endl;

    std::cout << "       budget debt [list]                                         Display the unpaid debts" << std::endl;
    std::cout << "       budget debt all                                            Display all debts" << std::endl;
    std::cout << "       budget debt paid (id)                                      Mark the given debt as paid" << std::endl;
    std::cout << "       budget debt delete (id)                                    Delete the given debt" << std::endl;
    std::cout << "       budget debt edit (id)                                      Edit the given debt" << std::endl << std::endl;

    std::cout << "       budget overview [month]                                    Display the overvew of the current month" << std::endl;
    std::cout << "       budget overview month (month) (year)                       Display the overvew of the specified month of the current year" << std::endl;
    std::cout << "       budget overview year (year)                                Display the overvew of the specified year" << std::endl;
    std::cout << "       budget overview aggregate                                  Display the aggregated expenses for the current year" << std::endl;
    std::cout << "       budget overview aggregate year (year)                      Display the aggregated expenses for the given year" << std::endl << std::endl;

    std::cout << "       budget report [monthly]                                    Display monthly report in form of bar plot" << std::endl;
}
