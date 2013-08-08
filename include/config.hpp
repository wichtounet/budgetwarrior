//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONFIG_H
#define CONFIG_H

namespace budget {

const std::size_t DATA_VERSION = 1;

std::string home_folder();
std::string budget_folder();
std::string path_to_home_file(const std::string& file);
std::string path_to_budget_file(const std::string& file);

bool load_config();
void save_config();

bool config_contains(const std::string& key);
std::string config_value(const std::string& key);

} //end of namespace budget

#endif
