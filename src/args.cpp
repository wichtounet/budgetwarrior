//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>
#include <string>
#include <cstring>

#include "args.hpp"
#include "budget_exception.hpp"

std::vector<std::wstring> budget::parse_args(int argc, const char* argv[]){
    std::vector<std::wstring> args;

    for(int i = 0; i < argc - 1; ++i){
        wchar_t* buf = new wchar_t[strlen(argv[i+1])];
        size_t num_chars = mbstowcs(buf, argv[i+1], strlen(argv[i+1]));
        args.push_back(std::wstring(buf, num_chars));
        delete buf;
    }

    return std::move(args);
}

void budget::enough_args(const std::vector<std::wstring>& args, std::size_t min){
    if(args.size() < min){
        throw budget_exception(L"Not enough args for this command. Use budget help to see how the command should be used.");
    }
}
