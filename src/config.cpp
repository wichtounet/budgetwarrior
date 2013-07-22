//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <fstream>
#include <unordered_map>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <boost/algorithm/string.hpp>

#include <boost/filesystem.hpp>

#include "config.hpp"

using namespace budget;

static std::unordered_map<std::string, std::string> configuration;

bool budget::load_config(){
    auto config_path = path_to_home_file(".budgetrc");

    if(boost::filesystem::exists(config_path)){
        std::ifstream file(config_path);

        if(file.is_open()){
            if(file.good()){
                std::string line;
                while(file.good() && getline(file, line)){
                    std::vector<std::string> parts;
                    boost::split(parts, line, boost::is_any_of("="), boost::token_compress_on);

                    if(parts.size() != 2){
                        std::cout << "The configuration file " << config_path << " is invalid only supports entries in form of key=value" << std::endl;

                        return false;
                    }

                    configuration[parts[0]] = parts[1];
                }
            }
        }
    }

    return verify_folder();
}

std::string budget::budget_folder(){
    if(config_contains("directory")){
        return config_value("directory");
    }

    return path_to_home_file(".budget");
}

void create_version_file(){
    auto file_path = path_to_budget_file("version");

    std::ofstream file(file_path);
    file << budget::DATA_VERSION  << std::endl;
}

bool budget::verify_folder(){
    auto folder_path = budget_folder();

    if(!boost::filesystem::exists(folder_path)){
        std::cout << "The folder " << folder_path << " does not exist. Would like to create it [yes/no] ? ";

        std::string answer;
        std::cin >> answer;

        if(answer == "yes" || answer == "y"){
            if(boost::filesystem::create_directories(folder_path)){
                std::cout << "The folder " << folder_path << " was created. " << std::endl;

                create_version_file();

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
    return budget_folder() + "/" + file;
}

bool budget::config_contains(const std::string& key){
    return configuration.find(key) != configuration.end();
}

std::string budget::config_value(const std::string& key){
    return configuration[key];
}
