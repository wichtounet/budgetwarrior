//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "help.hpp"

void budget::help_module::handle(const std::vector<std::string>&){
    std::cout << "Usage: budget                                          Display the overview of the current month\n\n";

    std::cout << "       budget account [show]                           Show the current accounts\n";
    std::cout << "       budget account all                              Show all the accounts\n";
    std::cout << "       budget account add                              Add a new account\n";
    std::cout << "       budget account delete (id)                      Delete the given account\n";
    std::cout << "       budget account edit (id)                        Edit the given account\n";
    std::cout << "       budget account transfer                         Transfer money from one account to another\n";
    std::cout << "       budget account migrate                          Merge two accounts together\n";
    std::cout << "       budget account archive (month)                  Archive the current accounts to start new accounts from this month\n\n";
    std::cout << "       budget account archive (year)                   Archive the current accounts to start new accounts from this year\n\n";

    std::cout << "       budget expense [show]                           Display the expenses of the current month\n";
    std::cout << "       budget expense show (month) (year)              Display the expenses of the specified month of the specified year\n";
    std::cout << "       budget expense all                              Display all the expenses\n";
    std::cout << "       budget expense add                              Add a new expense\n";
    std::cout << "       budget expense add (template name)              Add a new expense from a template or create a new template\n";
    std::cout << "       budget expense delete (id)                      Remove completely the expense with the given id\n";
    std::cout << "       budget expense edit (id)                        Modify the expense with the given id\n";
    std::cout << "       budget expense template                         Display the templates\n\n";

    std::cout << "       budget earning [earnings]                       Display the earnings of the current month\n";
    std::cout << "       budget earning show (month) (year)              Display the earnings of the specified month of the specified year\n";
    std::cout << "       budget earning all                              Display all the earnings\n";
    std::cout << "       budget earning add                              Add a new earning\n";
    std::cout << "       budget earning delete (id)                      Remove completely the earning with the given id\n";
    std::cout << "       budget earning edit (id)                        Modify the earning with the given id\n\n";

    std::cout << "       budget summary [month]                          Display a complete summary of the current month and the state of the current year\n";
    std::cout << "       budget summary [month] (month)                  Display a complete summary of the given month and the state of the given year\n\n";

    std::cout << "       budget overview [month]                         Display the overvew of the current month\n";
    std::cout << "       budget overview month (month) (year)            Display the overvew of the specified month of the current year\n";
    std::cout << "       budget overview year (year)                     Display the overvew of the specified year\n";
    std::cout << "       budget overview aggregate                       Display the aggregated expenses for the current year\n";
    std::cout << "       budget overview aggregate year (year)           Display the aggregated expenses for the given year\n\n";

    std::cout << "       budget recurring [show]                         Display the recurring expenses\n";
    std::cout << "       budget recurring add                            Add a new recurring expense\n";
    std::cout << "       budget recurring delete (id)                    Remove completely the recurring expense with the given id\n";
    std::cout << "       budget recurring edit (id)                      Modify the recurring expense with the given id\n\n";

    std::cout << "       budget fortune [status]                         Display the status of the fortune checks\n";
    std::cout << "       budget fortune list                             Display the list of all the fortune checks\n";
    std::cout << "       budget fortune check                            Create a new fortune check\n";
    std::cout << "       budget fortune edit (id)                        Edit a fortune check\n";
    std::cout << "       budget fortune delete (id)                      Delete a fortune check\n\n";

    std::cout << "       budget asset [show]                             Display the assets\n";
    std::cout << "       budget asset add                                Add a new asset\n";
    std::cout << "       budget asset add edit (id)                      Edit an asset\n";
    std::cout << "       budget asset add delete (id)                    Delete an asset\n";
    std::cout << "       budget asset value [show]                       Display the asset values (net worth)\n";
    std::cout << "       budget asset value list                         Display a list of the asset values\n";
    std::cout << "       budget asset value add                          Set the value of an list\n";
    std::cout << "       budget asset value edit it                      Edits an asset value\n";
    std::cout << "       budget asset value delete it                    Delete an asset value\n";

    std::cout << "       budget objective [status]                       Show the status of the objectives for the current year\n";
    std::cout << "       budget objective list                           List all the defined objectives\n";
    std::cout << "       budget objective add                            Add a new objective\n";
    std::cout << "       budget objective edit (id)                      Edit an objective\n";
    std::cout << "       budget objective delete (id)                    Delete an objective\n\n";

    std::cout << "       budget wish [status]                            Show the status of the wishes\n";
    std::cout << "       budget wish list                                List all the wishes\n";
    std::cout << "       budget wish estimate                            Estimate the best date to buy something\n";
    std::cout << "       budget wish add                                 Add a new wish\n";
    std::cout << "       budget wish edit (id)                           Edit a wish\n";
    std::cout << "       budget wish delete (id)                         Delete a wish\n\n";

    std::cout << "       budget debt [list]                              Display the unpaid debts\n";
    std::cout << "       budget debt all                                 Display all debts\n";
    std::cout << "       budget debt paid (id)                           Mark the given debt as paid\n";
    std::cout << "       budget debt delete (id)                         Delete the given debt\n";
    std::cout << "       budget debt edit (id)                           Edit the given debt\n\n";

    std::cout << "       budget report [monthly]                         Display monthly report in form of bar plot\n";
    std::cout << "       budget report account [monthly]                 Display monthly report of a specific account in form of bar plot\n\n";

    std::cout << "       budget versioning save                          Commit the budget directory changes with Git\n";
    std::cout << "       budget versioning sync                          Pull the remote changes on the budget directory with Git and push\n";
    std::cout << "       budget sync                                     Pull the remote changes on the budget directory with Git and push\n\n";

    std::cout << "       budget gc                                       Make sure all IDs are contiguous\n";
}
