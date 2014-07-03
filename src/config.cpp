//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
#include "utils.hpp"

using namespace budget;

typedef std::unordered_map<std::string, std::string> config_type;

namespace {

bool load_configuration(const std::string& path, config_type& configuration){
    if(file_exists(path)){
        std::ifstream file(path);

        if(file.is_open()){
            if(file.good()){
                std::string line;
                while(file.good() && getline(file, line)){
                    std::vector<std::string> parts;
                    boost::split(parts, line, boost::is_any_of("="), boost::token_compress_on);

                    if(parts.size() != 2){
                        std::cout << "The configuration file " << path << " is invalid only supports entries in form of key=value" << std::endl;

                        return false;
                    }

                    configuration[parts[0]] = parts[1];
                }
            }
        }
    }

    return true;
}

void save_configuration(const std::string& path, const config_type& configuration){
    std::ofstream file(path);

    for(auto& entry : configuration){
        file << entry.first << "=" << entry.second << std::endl;
    }
}

bool verify_folder(){
    auto folder_path = budget_folder();

    if(!folder_exists(folder_path)){
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

} //end of anonymous namespace

static config_type configuration;
static config_type internal;
static config_type internal_bak;

bool budget::load_config(){
    if(!load_configuration(path_to_home_file(".budgetrc"), configuration)){
        return false;
    }

    if(!verify_folder()){
        return false;
    }

    if(!load_configuration(path_to_budget_file("config"), internal)){
        return false;
    }

    internal_bak = internal;

    //At the first start, the version is not set
    if(internal.find("data_version") == internal.end()){
        internal["data_version"] = budget::to_string(budget::DATA_VERSION);
    }

    return true;
}

void budget::save_config(){
    if(internal != internal_bak){
        save_configuration(path_to_budget_file("config"), internal);
    }
}

std::string budget::home_folder(){
    struct passwd *pw = getpwuid(getuid());

    const char* homedir = pw->pw_dir;

    return std::string(homedir);
}

std::string budget::path_to_home_file(const std::string& file){
    return home_folder() + "/" + file;
}

std::string budget::budget_folder(){
    if(config_contains("directory")){
        return config_value("directory");
    }

    return path_to_home_file(".budget");
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

bool budget::internal_config_contains(const std::string& key){
    return internal.find(key) != internal.end();
}

std::string& budget::internal_config_value(const std::string& key){
    return internal[key];
}
