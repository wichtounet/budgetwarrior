budgetwarrior 1.1.0
===================

.. image:: https://github.com/wichtounet/budgetwarrior/actions/workflows/make.yml/badge.svg
   :target: https://github.com/wichtounet/budgetwarrior/actions/workflows/make.yml
   :alt: Linux Build

.. image:: https://github.com/wichtounet/budgetwarrior/actions/workflows/cmake.yml/badge.svg
   :target: https://github.com/wichtounet/budgetwarrior/actions/workflows/cmake.yml
   :alt: Mac OS Build

.. image:: https://github.com/wichtounet/budgetwarrior/actions/workflows/sonar.yml/badge.svg
   :target: https://github.com/wichtounet/budgetwarrior/actions/workflows/sonar.yml
   :alt: Sonar Build

.. image:: https://sonarcloud.io/api/project_badges/measure?project=wichtounet_budgetwarrior&metric=alert_status
   :target: https://sonarcloud.io/summary/new_code?id=wichtounet_budgetwarrior
   :alt: SonarSource analysis

.. image:: https://img.shields.io/badge/License-MIT-yellow.svg
   :target: https://opensource.org/licenses/MIT
   :alt: MIT License

Simple command line utility to helps keeping track of your expenses and the
state of your budget.

.. image:: https://github.com/wichtounet/budgetwarrior/blob/master/screenshots/budget_report.png?raw=true
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
And the tool will indicate you how well your accounts are doing, month by month and by year.

Features
--------

budgetwarrior has several features:

* Manages a set of account with a monthly limit
* Keeps track of your expenses and earnings in each of your accounts
* Gives you the state of your budget by month and by year
* Keeps track of your debts
* Manages recurring expenses that are automatically created monthly
* Keeps track of your wishes
* Keeps track of your current fortune

On the other hand, budgetwarrior:

* Has no notion of bank, cash, credit card
* Will not communicate with your ebanking tool to extract information

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

A modern compiler is necessary. I am working with GCC 13 and clang 17. You need to set the
$CXX variable before executing make.

Linux
=====

The tool is developed for Linux.  You need libcurl and libuuid installed on your computer to build this project.

You just have to use make to build it::

    $ git clone --recursive git://github.com/wichtounet/budgetwarrior.git
    $ cd budgetwarrior
    $ make
    $ sudo make install

If you want to install the man pages and the shell completion files, you can
do so with::

    $ sudo make install_extra

MacOS
=====

MacOS support is experimental.  Using CMake and Homebrew, you can build from sources.

1. First, install Xcode compiler toolchain via `xcode-select --install` if needed.

2. Use Homebrew to install required dependencies. Download and install brew from
   https://docs.brew.sh if you don't already have it::

    $ brew install cmake
    $ brew install git
    $ brew install openssl

Now build using the provided script after cloning repository::
	
    $ git clone --recursive git://github.com/wichtounet/budgetwarrior.git
    $ cd budgetwarrior
    $ ./build_macos.sh

To install the resulting, compiled program::

    $ cd ./build && make install

Windows
=======

If there is some interest on using it on Windows, it
should not take too much work to port it to Windows. Just let me know (or
make a Pull Request with the changes ;) ). .

Usage
-----

Man pages and ZSH/Bash completions are also available.

The executable is named 'budget' and allows to perform all the commands.

Use::

    $ budget help

Or::

    $ man budget

if you have installed the man pages to see all the available commands.

If you want to use the web interface, you will need to use another project: https://github.com/wichtounet/budgetwarrior_web

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

Troubleshooting
---------------

Please consider using `Github issues tracker <http://github.com/wichtounet/budgetwarrior/issues>`_ to submit bug reports or feature requests. You can also contact me via my `website <http://baptiste-wicht.com/>`_.

License
-------

This project is distributed under the MIT License. Read *LICENSE* for details.
