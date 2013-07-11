//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <boost/filesystem.hpp>

#include "config.hpp"

using namespace budget;

bool budget::verify_folder(){
    auto folder_path = path_to_home_file(".budget");

    if(!boost::filesystem::exists(folder_path)){
        std::cout << "The folder " << folder_path << " does not exist. Would like to create it [yes/no] ? ";

        std::string answer;
        std::cin >> answer;

        if(answer == "yes" || answer == "y"){
            if(boost::filesystem::create_directories(folder_path)){
                std::cout << "The folder " << folder_path << " was created. " << std::endl;

                return true;
            } else {
                std::cout << "Impossible to create the folder " << folder_path << std::endl;

                return false;
            }
        } else {
            return false;
        }
    }

    return true;
}

std::string budget::home_folder(){
    struct passwd *pw = getpwuid(getuid());

    const char* homedir = pw->pw_dir;

    return std::string(homedir);
}

std::string budget::path_to_home_file(const std::string& file){
    return home_folder() + "/" + file;
}

std::string budget::path_to_budget_file(const std::string& file){
    return path_to_home_file(".budget/" + file);
}
