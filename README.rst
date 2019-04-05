budgetwarrior 1.1.0
===================

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

A modern compiler is necessary: GCC >= 4.7 or Clang >= 3.1 You need to set the
$CXX variable before executing make.

Linux
=====

The tool is developed for Linux.  You need libcurl and libuuid installed on your computer to build this project.

You just have to use make to build it::

    $ git clone --recursive git://github.com/wichtounet/budgetwarrior.git
    $ cd budgetwarrior
    $ make
    $ sudo make install

MacOS
=====

Using CMake and brew, build from sources.  First, use brew to install required dependencies.  Assumes
you have Xcode command via `xcode-select --install` first. Next download and install brew
from https://docs.brew.sh

Then you can insstall cmake, git and openssl:

    $ brew install cmake
	$ brew install git
	$ brew install openssl

Now build using script after cloning repository:	
	
    $ git clone --recursive git://github.com/wichtounet/budgetwarrior.git
    $ cd budgetwarrior
    $ ./build_macos.sh
    $ make

	
Windows
=======

If there is some interest on using it on Windows, it
should not take too much work to port it to Windows. Just le met me know (or
make a Pull Request with the changes ;) ). .

Usage
-----

Man pages and ZSH/Bash completions are also available.

The executable is named 'budget' and allows to perform all the commands.

Use::

    $ budget help

Or::

    $ man budget

if you have installed the man pages.

to see all the available commands.

If you want to use the web interface, you can use::

    $ budget server

You can then go to localhost:8080 to view and edit your budget online. The
default user is admin and the default password is 1234. You can edit them in the
configuration file::

    web_user=admin
    web_password=1234

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
