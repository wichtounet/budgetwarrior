budgetwarrior 0.4.1.2
=====================

Simple command line utility to helps keeping track of your expenses and the
state of your budget.

.. image:: https://raw.githubusercontent.com/wichtounet/budgetwarrior/develop/screenshots/budget_report.png
   :align: center
   
Personal accounting method
--------------------------

Not everyone manages its money the same way. budgetwarrior is based on my way of managing my money. It may not be adapted to everyone. I'm separating all my expenses in several "accounts" and I give each of them a certain amount of money each month. 

**Accounts are recipients for your expenses and earnings. They are not bank accounts, they only are logical recipients**. Think of them as categories. The sum of the accounts should be equivalent to your salary.

For instance, if you have 1000$ per month, you could have three accounts: 
* Food (200$)
* Car (300$)
* House (500$)

That means you allow yourself to spend 200$ on food each month. Of course, some month you'll be lower or higher. 
And the tool will indicate you how well your accounts. 

Features
--------

budgetwarrior has several features:

* Manage a set of account with a monthly limit
* Keeps track of your expenses and earnings in each of your accounts
* Give you the state of your budget by month and by year
* Keeps track of your debts
* Manage recurring expenses that are automatically created monthly
* Keep track of your wishes
* Keep track of your current fortune

On the other hand, budgetwarrior:

* Has no notion of bank, cash, credit card
* Will not communicate with your ebanking tool to extract information

Start using budgetwarrior
-------------------------

The wiki contains a guide for starting using budgetwarrior: `Start-Tutorial <https://github.com/wichtounet/budgetwarrior/wiki/Start-tutorial>`_

Installation
------------

A Gentoo ebuild is available on this overlay: https://github.com/wichtounet/wichtounet-overlay

Arch Linux packages are also available on AUR: https://github.com/StreakyCobra/aur

For other systems, you'll have to install from sources. 

Build from source
-----------------

A modern compiler is necessary: GCC >= 4.7 or CLang >= 3.1.

The tool is made for Linux. If there is some interest on using it on Windows, it
should not take too much work to port it to Windows. Just le met me know (or
make a Pull Request with the changes ;) ). . 

You need Boost 1.47.0 or superior and libuuid installed on your computer
to build this project.

You juste have to use make to build it::

    $ git clone git://github.com/wichtounet/budgetwarrior.git
    $ cd budgetwarrior
    $ make
    $ sudo make install

Man pages and ZSH/Bash completions are also available.

Usage
-----

The executable is named 'budget' and allows to perform all the commands.

Use::

    $ budget help

Or::

    $ man budget

if you have installed the man pages.

to see all the available commands.

Contributors
------------

Read *AUTHORS*

Release Notes
-------------

Read *ChangeLog*

Contribute
----------

The project is open for any kind of contribution : ideas, new features, hotfixes, tests, ...

If you want to contribute to this project, you can contact me by `email <baptiste.wicht@gmail.com>`_ or via my `website  <http://baptiste-wicht.com/>`_. You can also directly fork the project and make a pull request.

If you want to support the development of this project, you can `donate via Pledgie <http://pledgie.com/campaigns/21113>`_. Thank you !

Troubleshooting
---------------

Please consider using `Github issues tracker <http://github.com/wichtounet/budgetwarrior/issues>`_ to submit bug reports or feature requests. You can also contact me via my `website <http://baptiste-wicht.com/>`_.

License
-------

This project is distributed under the MIT License. Read *LICENSE* for details.
