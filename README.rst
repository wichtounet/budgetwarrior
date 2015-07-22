budgetwarrior 0.4.1.2
=====================

Simple command line utility to helps keeping track of your expenses and the
state of your budget.

.. image:: https://raw.githubusercontent.com/wichtounet/budgetwarrior/develop/screenshots/budget_report.png
   :align: center

Features
--------

budgetwarrior has several features:

* Manages a set of account with a limit
* Keeps track of your expenses and earnings in each of your accounts
* Gives you the state of your budget by month and by year
* Keeps tracks of your objectives
* Keeps track of your wish list and indicates if it is a good time to buy them
* Manages recurring expenses that are automatically created monthly
* Keeps track of your debts (your debts and debts that people owe you)

Starting guide
-------------

The wiki contains a guide for starting using budgetwarrior: `Start-Tutorial <https://github.com/wichtounet/budgetwarrior/wiki/Start-tutorial>`_

Installation
------------

A Gentoo ebuild is available on this overlay: https://github.com/wichtounet/wichtounet-overlay

Arch Linux packages are also available on AUR: https://github.com/StreakyCobra/aur

For other systems, you'll have to install from sources.

Build from source
-----------------

A modern compiler is necessary: GCC >= 4.7 or Clang >= 3.1.

The tool is made for Linux. If there is some interest on using it on Windows, it
should not take too much work to port it to Windows. Just le met me know (or
make a Pull Request with the changes ;) ). .

You need Boost 1.47.0 or superior and libuuid installed on your computer
to build this project.

You just have to use make to build it::

    $ git clone --recursive git://github.com/wichtounet/budgetwarrior.git
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

The project is open for any kind of contribution : ideas, new features, bug fixes, tests, ...

If you want to contribute to this project, you can contact me by `email <baptiste.wicht@gmail.com>`_ or via my `website  <http://baptiste-wicht.com/>`_. You can also directly fork the project and make a pull request.

If you want to support the development of this project, you can `donate via Pledgie <http://pledgie.com/campaigns/21113>`_. Thank you !

Troubleshooting
---------------

Please consider using `Github issues tracker <http://github.com/wichtounet/budgetwarrior/issues>`_ to submit bug reports or feature requests. You can also contact me via my `website <http://baptiste-wicht.com/>`_.

License
-------

This project is distributed under the MIT License. Read *LICENSE* for details.
