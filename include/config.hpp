//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CONFIG_H
#define CONFIG_H

namespace budget {

const std::size_t DATA_VERSION = 3;

std::string home_folder();
std::string budget_folder();
std::string path_to_home_file(const std::string& file);
std::string path_to_budget_file(const std::string& file);

bool load_config();
void save_config();

bool config_contains(const std::string& key);
std::string config_value(const std::string& key);

bool internal_config_contains(const std::string& key);
std::string& internal_config_value(const std::string& key);

} //end of namespace budget

#endif
