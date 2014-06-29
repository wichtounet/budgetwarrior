//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "utils.hpp"
#include "budget_exception.hpp"
#include "assert.hpp"
#include "config.hpp"
#include "expenses.hpp"
#include "earnings.hpp"

#include <cstdio>
#include <sys/ioctl.h>
#include <unistd.h>

unsigned short budget::terminal_width(){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

unsigned short budget::terminal_height(){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}
