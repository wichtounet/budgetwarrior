//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <unordered_map>

#include <unistd.h>    //for getuid
#include <sys/types.h> //for getuid
#include <sys/stat.h>  //For mkdir
#ifdef _WIN32
#include <shlobj.h>
#else
#include <pwd.h>       //For getpwuid
#endif

#include "config.hpp"
#include "utils.hpp"
#include "logging.hpp"

#include "assets.hpp"
#include "fortune.hpp"
#include "server_lock.hpp"

#include <filesystem>
namespace fs = std::filesystem;

using namespace budget;

using config_type = std::unordered_map<std::string, std::string>;

namespace {

bool server_running = false;

budget::server_lock internal_config_lock;

bool load_configuration(const std::string& path, config_type& configuration){
    if (file_exists(path)) {
        std::ifstream file(path);

        LOG_F(INFO, "Load configuration from {}", path);

        if (file.is_open() && file.good()) {
            std::string line;
            while (file.good() && getline(file, line)) {
                // Ignore empty lines
                if (line.empty()) {
                    continue;
                }

                // Ignore comments
                if(line[0] == '#'){
                    continue;
                }

                auto first = line.find('=');

                if (first == std::string::npos || line.rfind('=') != first) {
                    LOG_F(ERROR, "The configuration file file {} is invalid, only supports key=value entry", path);

                    return false;
                }

                auto key   = line.substr(0, first);
                auto value = line.substr(first + 1, line.size());

                configuration[key] = value;
            }
        } else {
            LOG_F(ERROR, "Unable to open config file {}", path);
        }
    }

    return true;
}

void save_configuration(const std::string& path, const config_type& configuration){
    std::ofstream file(path);

    for(auto& [key, value] : configuration){
        file << key << "=" << value << std::endl;
    }
}

bool verify_folder(){
    auto folder_path = budget_folder();

    LOG_F(INFO, "Using {} as data directory", folder_path);

    if (!folder_exists(folder_path)) {
        std::cout << "The folder " << folder_path << " does not exist. Would like to create it [yes/no] ? ";

        std::string answer;
        std::cin >> answer;

        if(answer == "yes" || answer == "y"){
#ifdef _WIN32
            if(mkdir(folder_path.c_str()) == 0){
#else
            if(mkdir(folder_path.c_str(), ACCESSPERMS) == 0){
#endif
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

bool budget::load_config() {
    if(!load_configuration(config_file(), configuration)){
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

void budget::save_config() {
    if (internal != internal_bak) {
        server_lock_guard l(internal_config_lock);

        save_configuration(path_to_budget_file("config"), internal);

        internal_bak = internal;

        LOG_F(INFO, "Save internal configuration");
    }
}

std::string budget::config_file() {
    auto old_config = path_to_home_file(".budgetrc");
    if(file_exists(old_config)) {
        return old_config;
    }

    fs::path config_home;
    if(auto xdg_config_home = std::getenv("XDG_CONFIG_HOME")) {
        config_home = fs::path{xdg_config_home};
    } else {
        config_home = fs::path{home_folder()} / ".config";
    }

    return (config_home / "budget" / "budgetrc").string();
}

std::string budget::home_folder() {
#ifdef _WIN32
    TCHAR path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        std::wstring wpath(path);
        return std::string(wpath.begin(), wpath.end());
    }
    return "c:\\";
#else
    struct passwd *pw = getpwuid(getuid());

    const char* homedir = pw->pw_dir;

    return std::string(homedir);
#endif
}

std::string budget::budget_folder() {
    if(config_contains("directory")) {
        return config_value("directory");
    }

    auto old_home = path_to_home_file(".budget");
    if(file_exists(old_home)) {
        return old_home;
    }

    if(auto data_home = std::getenv("XDG_DATA_HOME")) {
        return (fs::path{data_home} / "budget").string();
    }

    return home_folder() + "/.local/share/budget";
}
 
std::string budget::path_to_home_file(const std::string& file) {
    return home_folder() + "/" + file;
}

std::string budget::path_to_budget_file(const std::string& file) {
    return budget_folder() + "/" + file;
}

bool budget::config_contains(const std::string& key){
    return configuration.find(key) != configuration.end();
}

std::string budget::config_value(const std::string& key){
    return configuration[key];
}

std::string budget::config_value(const std::string& key, const std::string& def){
    if (config_contains(key)) {
        return config_value(key);
    }

    return def;
}

bool budget::config_contains_and_true(const std::string& key) {
    if (config_contains(key)) {
        return config_value(key) == "true";
    }

    return false;
}

std::string budget::user_config_value(const std::string& key, const std::string& def) {
    // 1. Check for the global configuration
    if (config_contains(key)) {
        return config_value(key);
    }

    // 2. Check foir the internal configuration
    if (internal_config_contains(key)) {
        return internal_config_value(key);
    }

    // 3. Return the default value
    return def;
}

bool budget::user_config_value_bool(const std::string& key, bool def) {
    // 1. Check for the global configuration
    if (config_contains(key)) {
        return config_value(key) == "true";
    }

    // 2. Check foir the internal configuration
    if (internal_config_contains(key)) {
        return internal_config_value(key) == "true";
    }

    // 3. Return the default value
    return def;
}

bool budget::internal_config_contains(const std::string& key){
    server_lock_guard l(internal_config_lock);
    return internal.find(key) != internal.end();
}

std::string budget::internal_config_value(const std::string& key){
    server_lock_guard l(internal_config_lock);
    return internal[key];
}

void budget::internal_config_set(const std::string& key, const std::string & value){
    server_lock_guard l(internal_config_lock);
    internal[key] = value;
}

void budget::internal_config_remove(const std::string& key){
    server_lock_guard l(internal_config_lock);
    internal.erase(key);
}

std::string budget::get_web_user(){
    return user_config_value("web_user", "admin");
}

std::string budget::get_web_password(){
    return user_config_value("web_password", "1234");
}

std::string budget::get_server_listen(){
    if (config_contains("server_listen")) {
        return config_value("server_listen");
    }

    return "localhost";
}

size_t budget::get_server_port(){
    if (config_contains("server_port")) {
        return to_number<size_t>(config_value("server_port"));
    }

    return 8080;
}

bool budget::is_server_mode(){
    // The server cannot run in server mode
    if (is_server_running()) {
        return false;
    }

    return config_contains_and_true("server_mode");
}

bool budget::is_secure(){
    if (config_contains("server_secure")) {
        return config_value("server_secure") != "false";
    }

    return true;
}

bool budget::is_server_ssl(){
    if (config_contains("server_ssl")) {
        return config_value("server_ssl") == "true";
    }

    return config_contains_and_true("server_ssl");
}

bool budget::is_fortune_disabled(){
    return user_config_value_bool("disable_fortune", false);
}

bool budget::is_debts_disabled(){
    return user_config_value_bool("disable_debts", false);
}

bool budget::net_worth_over_fortune(){
    // If the fortune module is disabled, use net worth
    if (is_fortune_disabled()) {
        return true;
    }

    // By default, fortune is the thing being taken into account
    // Unless it's not used and net worth is used

    // TODO This can be a very expensive operation!
    return !no_asset_values() && no_fortunes();
}

void budget::set_server_running(){
    // Indicates to the system that it's running in server mode
    server_running = true;
}

bool budget::is_server_running(){
    return server_running;
}
