//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>
#include <string>

#include "args.hpp"
#include "budget_exception.hpp"

std::vector<std::string> budget::parse_args(int argc, const char* argv[]){
    std::vector<std::string> args;

    for(int i = 0; i < argc - 1; ++i){
        args.push_back(std::string(argv[i+1]));
    }

    return std::move(args);
}

void budget::enough_args(const std::vector<std::string>& args, std::size_t min){
    if(args.size() < min){
        throw budget_exception("Not enough args for this command. Use budget help to see how the command should be used.");
    }
}
