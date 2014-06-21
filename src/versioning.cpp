//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <sstream>
#include <fstream>

#include "versioning.hpp"
#include "budget_exception.hpp"
#include "config.hpp"

namespace {

std::string exec_command(const std::string& command) {
    std::stringstream output;

    char buffer[1024];

    FILE* stream = popen(command.c_str(), "r");

    while (fgets(buffer, 1024, stream) != NULL) {
        output << buffer;
    }

    pclose(stream);

    return output.str();
}

} //end of anonymous namespace

void budget::versioning_module::handle(const std::vector<std::string>& args){
    if(args.size() == 1){
        std::cout << "Missing subcommand" << std::endl;
    } else {
        auto& subcommand = args[1];

        if(subcommand == "save"){
            std::cout << exec_command("git -C " + budget_folder() + " commit -a -m Update" ) << std::endl;
        } else if(subcommand == "sync"){
            std::cout << exec_command("git -C " + budget_folder() + " commit -a -m Update" ) << std::endl;
            std::cout << exec_command("git -C " + budget_folder() + " pull" ) << std::endl;
            std::cout << exec_command("git -C " + budget_folder() + " push" ) << std::endl;
        } else if(subcommand == "pull"){
            std::cout << exec_command("git -C " + budget_folder() + " pull" ) << std::endl;
        } else if(subcommand == "push"){
            std::cout << exec_command("git -C " + budget_folder() + " push" ) << std::endl;
        } else if(subcommand == "status"){
            std::cout << exec_command("git -C " + budget_folder() + " status" ) << std::endl;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}
