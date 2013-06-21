//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONFIG_H
#define CONFIG_H

namespace budget {

std::string home_folder();
std::string path_to_home_file(const std::string& file);

bool verify_folder();

} //end of namespace budget

#endif
