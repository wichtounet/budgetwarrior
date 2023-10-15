//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <vector>
#include <string>

#include "args.hpp"
#include "budget_exception.hpp"
#include "utils.hpp"

std::vector<std::string> budget::parse_args(int argc, const char* argv[], const std::vector<std::pair<const char*, const char*>>& aliases){
    std::vector<std::string> args;

    for(int i = 0; i < argc - 1; ++i){
        const std::string arg(argv[i + 1]);

        if(i == 0){
            bool found_alias = false;

            for (const auto& [alias_source, alias_target] : aliases) {
                if(alias_source == arg){
                    auto parts = split(alias_target, ' ');

                    std::ranges::copy(parts, std::back_inserter(args));

                    found_alias = true;
                }
            }

            if(found_alias){
                continue;
            }
        }

        args.push_back(arg);
    }

    return args;
}

void budget::enough_args(const std::vector<std::string>& args, size_t min){
    if(args.size() < min){
        throw budget_exception("Not enough args for this command. Use budget help to see how the command should be used.");
    }
}
