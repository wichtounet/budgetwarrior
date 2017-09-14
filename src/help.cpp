//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
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
    std::cout << "       budget account transfer                                    Transfer money from one account to another" << std::endl << std::endl;
    std::cout << "       budget account migrate                                     Merge two accounts together" << std::endl << std::endl;
    std::cout << "       budget account archive                                     Archive the current accounts to start new accounts" << std::endl << std::endl;

    std::cout << "       budget expense [show]                                      Display the expenses of the current month" << std::endl;
    std::cout << "       budget expense show (month) (year)                         Display the expenses of the specified month of the specified year" << std::endl;
    std::cout << "       budget expense all                                         Display all the expenses" << std::endl;
    std::cout << "       budget expense add                                         Add a new expense" << std::endl;
    std::cout << "       budget expense add (template name)                         Add a new expense from a template or create a new template" << std::endl;
    std::cout << "       budget expense delete (id)                                 Remove completely the expense with the given id" << std::endl;
    std::cout << "       budget expense edit (id)                                   Modify the expense with the given id" << std::endl << std::endl;
    std::cout << "       budget expense template                                    Display the templates" << std::endl << std::endl;

    std::cout << "       budget earning [earnings]                                  Display the earnings of the current month" << std::endl;
    std::cout << "       budget earning show (month) (year)                         Display the earnings of the specified month of the specified year" << std::endl;
    std::cout << "       budget earning all                                         Display all the earnings" << std::endl;
    std::cout << "       budget earning add                                         Add a new earning" << std::endl;
    std::cout << "       budget earning delete (id)                                 Remove completely the earning with the given id" << std::endl;
    std::cout << "       budget earning edit (id)                                   Modify the earning with the given id" << std::endl << std::endl;

    std::cout << "       budget overview [month]                                    Display the overvew of the current month" << std::endl;
    std::cout << "       budget overview month (month) (year)                       Display the overvew of the specified month of the current year" << std::endl;
    std::cout << "       budget overview year (year)                                Display the overvew of the specified year" << std::endl;
    std::cout << "       budget overview aggregate                                  Display the aggregated expenses for the current year" << std::endl;
    std::cout << "       budget overview aggregate year (year)                      Display the aggregated expenses for the given year" << std::endl << std::endl;

    std::cout << "       budget recurring [show]                                    Display the recurring expenses" << std::endl;
    std::cout << "       budget recurring add                                       Add a new recurring expense" << std::endl;
    std::cout << "       budget recurring delete (id)                               Remove completely the recurring expense with the given id" << std::endl;
    std::cout << "       budget recurring edit (id)                                 Modify the recurring expense with the given id" << std::endl << std::endl;

    std::cout << "       budget fortune [status]                                    Display the status of the fortune checks" << std::endl;
    std::cout << "       budget fortune list                                        Display the list of all the fortune checks" << std::endl;
    std::cout << "       budget fortune check                                       Create a new fortune check" << std::endl;
    std::cout << "       budget fortune edit (id)                                   Edit a fortune check" << std::endl;
    std::cout << "       budget fortune delete (id)                                 Delete a fortune check" << std::endl << std::endl;

    std::cout << "       budget objective [status]                                  Show the status of the objectives for the current year" << std::endl;
    std::cout << "       budget objective list                                      List all the defined objectives" << std::endl;
    std::cout << "       budget objective add                                       Add a new objective" << std::endl;
    std::cout << "       budget objective edit (id)                                 Edit an objective" << std::endl;
    std::cout << "       budget objective delete (id)                               Delete an objective" << std::endl;

    std::cout << "       budget wish [status]                                       Show the status of the wishes" << std::endl;
    std::cout << "       budget wish list                                           List all the wishes" << std::endl;
    std::cout << "       budget wish estimate                                       Estimate the best date to buy something" << std::endl;
    std::cout << "       budget wish add                                            Add a new wish" << std::endl;
    std::cout << "       budget wish edit (id)                                      Edit a wish" << std::endl;
    std::cout << "       budget wish delete (id)                                    Delete a wish" << std::endl;

    std::cout << "       budget debt [list]                                         Display the unpaid debts" << std::endl;
    std::cout << "       budget debt all                                            Display all debts" << std::endl;
    std::cout << "       budget debt paid (id)                                      Mark the given debt as paid" << std::endl;
    std::cout << "       budget debt delete (id)                                    Delete the given debt" << std::endl;
    std::cout << "       budget debt edit (id)                                      Edit the given debt" << std::endl << std::endl;
    
    std::cout << "       budget versioning save                                     Commit the budget directory changes with Git" << std::endl;
    std::cout << "       budget versioning sync                                     Pull the remote changes on the budget directory with Git and push" << std::endl;

    std::cout << "       budget report [monthly]                                    Display monthly report in form of bar plot" << std::endl;
    std::cout << "       budget report account [monthly]                            Display monthly report of a specific account in form of bar plot" << std::endl;

    std::cout << "       budget gc                                                  Make sure all IDs are contiguous" << std::endl;
}
