//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "logging.hpp"

void budget::init_logging(int& argc, char** argv){
    // By default, we do not want to log arguments
    loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;

    loguru::init(argc, argv);
}

#include "loguru.cpp"
