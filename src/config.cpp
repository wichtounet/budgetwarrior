//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <filesystem>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "cpp_utils/hash.hpp"

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

namespace fs = std::filesystem;

using namespace budget;

namespace {

using config_type = cpp::string_hash_map<std::string>;

bool server_running = false;

budget::server_lock internal_config_lock;

bool load_configuration(const fs::path & path, config_type& configuration){
    if (fs::exists(path)) {
        std::ifstream file(path);

        LOG_F(INFO, "Load configuration from {}", path.string());

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
                    LOG_F(ERROR, "The configuration file file {} is invalid, only supports key=value entry", path.string());

                    return false;
                }

                auto key   = line.substr(0, first);
                auto value = line.substr(first + 1, line.size());

                configuration[key] = value;
            }
        } else {
            LOG_F(ERROR, "Unable to open config file {}", path.string());
        }
    }

    return true;
}

void save_configuration(const fs::path& path, const config_type& configuration){
    std::ofstream file(path);

    for (const auto& [key, value] : configuration) {
        file << key << "=" << value << std::endl;
    }
}

bool verify_folder(){
    auto folder_path = budget_folder();

    LOG_F(INFO, "Using {} as data directory", folder_path.string());

    if (!fs::is_directory(folder_path)) {
        std::cout << "The folder " << folder_path << " does not exist. Would like to create it [yes/no] ? ";

        std::string answer;
        std::cin >> answer;

        if(answer == "yes" || answer == "y"){
            std::error_code er;
            if (fs::create_directories(folder_path, er)) {
                std::cout << "The folder " << folder_path << " was created. " << std::endl;
                return true;
            }

            std::cout << "Impossible to create the folder " << folder_path << " error: " << er.message() << std::endl;
            return false;
        }

        return false;
    }

    return true;
}

config_type configuration;
config_type internal;
config_type internal_bak;

} //end of anonymous namespace

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
    if(!internal_config_contains("data_version")) {
        internal["data_version"] = budget::to_string(budget::DATA_VERSION);
    }

    return true;
}

void budget::save_config() {
    if (internal != internal_bak) {
        const server_lock_guard l(internal_config_lock);

        save_configuration(path_to_budget_file("config"), internal);

        internal_bak = internal;

        LOG_F(INFO, "Save internal configuration");
    }
}

fs::path budget::config_file() {
    if(auto old_config = path_to_home_file(".budgetrc"); fs::exists(old_config)) {
        return old_config;
    }

    fs::path config_home;
    if (auto* xdg_config_home = std::getenv("XDG_CONFIG_HOME")) {
        config_home = fs::path{xdg_config_home};
    } else {
        config_home = fs::path{home_folder()} / ".config";
    }

    return config_home / "budget" / "budgetrc";
}

fs::path budget::home_folder() {
#ifdef _WIN32
    TCHAR path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        std::wstring wpath(path);
        return std::string(wpath.begin(), wpath.end());
    }
    return fs::path{"c:\\"};
#else
    const struct passwd *pw = getpwuid(getuid());

    const char* homedir = pw->pw_dir;

    return fs::path{homedir};
#endif
}

fs::path budget::budget_folder() {
    if (auto it = configuration.find("directory"); it != configuration.end()) {
        return fs::path{it->second};
    }

    if(auto old_home = path_to_home_file(".budget"); fs::exists(old_home)) {
        return old_home;
    }

    if (auto* data_home = std::getenv("XDG_DATA_HOME")) {
        return fs::path{data_home} / "budget";
    }

    return home_folder() / "/.local/share/budget";
}
 
fs::path budget::path_to_home_file(std::string_view file) {
    return home_folder() / file;
}

fs::path budget::path_to_budget_file(std::string_view file) {
    return budget_folder() / file;
}

bool budget::config_contains(std::string_view key){
    return configuration.contains(key);
}

std::string budget::config_value(std::string_view key){
    auto it = configuration.find(key);
    cpp_assert(it != configuration.end(), "Cannot call config_value without making sure it's contained first");
    return it->second;
}

std::string budget::config_value(std::string_view key, std::string_view def){
    if (auto it = configuration.find(key); it != configuration.end()) {
        return it->second;
    }

    return std::string{def};
}

bool budget::config_contains_and_true(std::string_view key) {
    if (auto it = configuration.find(key); it != configuration.end()) {
        return it->second == "true";
    }

    return false;
}

std::string budget::user_config_value(std::string_view key, std::string_view def) {
    // 1. Check for the global configuration
    if (auto it = configuration.find(key); it != configuration.end()) {
        return it->second;
    }

    // 2. Check for the internal configuration
    if (auto it = internal.find(key); it != internal.end()) {
        return it->second;
    }

    // 3. Return the default value
    return std::string{def};
}

bool budget::user_config_value_bool(std::string_view key, bool def) {
    // 1. Check for the global configuration
    if (auto it = configuration.find(key); it != configuration.end()) {
        return it->second == "true";
    }

    // 2. Check foir the internal configuration
    if (auto it = internal.find(key); it != internal.end()) {
        return it->second == "true";
    }

    // 3. Return the default value
    return def;
}

bool budget::internal_config_contains(std::string_view key){
    const server_lock_guard l(internal_config_lock);
    return internal.contains(key);
}

std::string budget::internal_config_value(std::string_view key){
    auto it = internal.find(key);
    cpp_assert(it != internal.end(), "Cannot call internal_config_value without making sure it's contained first");
    return it->second;
}

void budget::internal_config_set(std::string_view key, std::string_view value){
    const server_lock_guard l(internal_config_lock);
    internal.insert_or_assign(std::string(key), value);
}

void budget::internal_config_remove(std::string_view key){
    const server_lock_guard l(internal_config_lock);
    auto              it = internal.find(key);
    if (it != internal.end()) {
        internal.erase(it);
    }
}

std::string budget::get_web_user(){
    return user_config_value("web_user", "admin");
}

std::string budget::get_web_password(){
    return user_config_value("web_password", "1234");
}

std::string budget::get_server_listen(){
    return config_value("server_listen", "localhost");
}

uint16_t budget::get_server_port(){
    return to_number<uint16_t>(config_value("server_port", "8080"));
}

bool budget::is_server_mode(){
    // The server cannot run in server mode
    if (is_server_running()) {
        return false;
    }

    return config_contains_and_true("server_mode");
}

bool budget::is_secure(){
    return config_contains_and_true("server_secure");
}

bool budget::is_server_ssl(){
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

    // This can be a very expensive operation! If this ever causes issues, I should find a better solution
    return !no_asset_values() && no_fortunes();
}

void budget::set_server_running(){
    // Indicates to the system that it's running in server mode
    server_running = true;
}

bool budget::is_server_running(){
    return server_running;
}
