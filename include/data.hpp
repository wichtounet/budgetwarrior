//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DATA_H
#define DATA_H

#include "config.hpp"

namespace budget {

template<typename T>
struct data_handler {
    std::size_t next_id;
    std::vector<T> data;
};

template<typename T>
void save_data(const data_handler<T>& data, const std::string& path){
    auto file_path = path_to_budget_file(path);

    std::ofstream file(file_path);
    file << data.next_id << std::endl;

    for(auto& entry: data.data){
        file << entry << std::endl;
    }
}

} //end of namespace budget

#endif
