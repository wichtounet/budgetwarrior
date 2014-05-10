budgetwarrior 0.3.0
===================

Simple command line utility to helps keeping track of your expenses and the
state of your budget.

## Features ##

budgetwarrior has several features:

* Manage a set of account with a limit
* Keeps track of your expenses and earnings in each of your accounts
* Give you the state of your budget by month and by year
* Keeps track of your debts
* Manage recurring expenses that are automatically created monthly

## Start using budgetwarrior ##

The wiki contains a guide for starting using budgetwarrior: https://github.com/wichtounet/budgetwarrior/wiki/Start-tutorial

## Installation ##

A Gentoo ebuild is available on this overlay: https://github.com/wichtounet/wichtounet-overlay

Arch Linux packages are also available: https://github.com/StreakyCobra/aur

For other systems, you'll have to install from sources. 

## Building from sources ##

A modern compiler is necessary: GCC >= 4.7 or CLang >= 3.1.

The tool is known to work on Linux but has never been tested on Windows.
You need Boost 1.47.0 or superior and libuuid installed on your computer
to build this project.

You juste have to use CMake to build it:

    $ git clone git://github.com/wichtounet/budgetwarrior.git
    $ cd budgetwarrior
    $ cmake .
    $ make
    $ sudo make install

Man pages and ZSH/Bash completions are also available.

## Usage ##

The executable is named 'budget' and allows to perform all the commands.

Use

    $ budget help

Or

    $ man budget

if you have installed the man pages.

to see all the available commands.

## Contributors ##

Read `AUTHORS`

## Release Notes ##

Read `ChangeLog`

## Contribute ##

The project is open for any kind of contribution : ideas, new features, hotfixes, tests, ...

If you want to contribute to this project, you can contact me by [email](baptiste.wicht@gmail.com) or via my [website](http://baptiste-wicht.com/). You can also directly fork the project and make a pull request.

If you want to support the development of this project, you can [donate via Pledgie](http://pledgie.com/campaigns/21113). Thank you !

## Troubleshooting ##

Please consider using [Github issues tracker](http://github.com/wichtounet/budgetwarrior/issues) to submit bug reports or feature requests. You can also contact me via my [website](http://baptiste-wicht.com/).

## License ##

This project is distributed under the MIT License. Read `LICENSE` for details.
