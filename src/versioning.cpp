//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
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
    // versioning does not make sense in server mode
    if (is_server_mode()) {
        std::cout << "The versioning commands are not available in server mode" << std::endl;
        return;
    }

    if(args.size() == 1){
        std::cout << "Missing subcommand" << std::endl;
    } else {
        auto& subcommand = args[1];

        if(subcommand == "save"){
            std::cout << exec_command("git -C " + budget_folder().string() + " commit -a -m Update" ) << std::endl;
        } else if(subcommand == "sync"){
            std::cout << exec_command("git -C " + budget_folder().string() + " commit -a -m Update" ) << std::endl;
            std::cout << exec_command("git -C " + budget_folder().string() + " pull" ) << std::endl;
            std::cout << exec_command("git -C " + budget_folder().string() + " push" ) << std::endl;
        } else if(subcommand == "pull"){
            std::cout << exec_command("git -C " + budget_folder().string() + " pull" ) << std::endl;
        } else if(subcommand == "push"){
            std::cout << exec_command("git -C " + budget_folder().string() + " push" ) << std::endl;
        } else if(subcommand == "status"){
            std::cout << exec_command("git -C " + budget_folder().string() + " status" ) << std::endl;
        } else {
            throw budget_exception("Invalid subcommand \"" + subcommand + "\"");
        }
    }
}
