//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <cstdlib>
#include <string>
#include <filesystem>

namespace budget {

const size_t MIN_DATA_VERSION = 4;
const size_t DATA_VERSION     = 9;

std::filesystem::path home_folder();
std::filesystem::path config_file();
std::filesystem::path budget_folder();
std::filesystem::path path_to_home_file(std::string_view file);
std::filesystem::path path_to_budget_file(std::string_view file);

bool load_config();
void save_config();

bool config_contains(std::string_view key);
std::string config_value(std::string_view key);
std::string config_value(std::string_view key, std::string_view def);
bool config_contains_and_true(std::string_view key);

std::string user_config_value(std::string_view key, std::string_view def);
bool user_config_value_bool(std::string_view key, bool def);

bool        internal_config_contains(std::string_view key);
std::string internal_config_value(std::string_view key);
void        internal_config_set(std::string_view key, std::string_view value);
void        internal_config_remove(std::string_view key);

std::string get_web_user();
std::string get_web_password();
std::string get_server_listen();
size_t get_server_port();

/*!
 * \brief Indicates if the server is running in secure mode.
 *
 * By default, the server is running in secure mode.
 * This can be changed by changing server_secure=false in the
 * configuration file.
 */
bool is_secure();

/*!
 * \brief Indicates if the application is running in server mode.
 *
 * Server mode indicates that the application is running in console
 * but directly sends requests to the server instead of saving to
 * files.
 */
bool is_server_mode();

/*!
 * \brief Indicates if the server is running with SSL.
 */
bool is_server_ssl();

/*!
 * \brief Indicates if the fortune module is disabled.
 */
bool is_fortune_disabled();

/*!
 * \brief Indicates if the debts module is disabled.
 */
bool is_debts_disabled();

/*!
 * \brief Indicates if net worth should be used instead of fortune
 * for computation.
 *
 * This needs fortunes and assets to be loaded
 */
bool net_worth_over_fortune();

void set_server_running();
bool is_server_running();

} //end of namespace budget
