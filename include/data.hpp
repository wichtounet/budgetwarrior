//=======================================================================
// Copyright Baptiste Wicht 2013.
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

    std::wofstream file(file_path.c_str());
    file << data.next_id << std::endl;

    for(auto& entry: data.data){
        file << entry << std::endl;
    }
}

template<typename T>
void load_data(data_handler<T>& data, const std::string& path){
    auto file_path = path_to_budget_file(path);

    if(!boost::filesystem::exists(file_path)){
        data.next_id = 1;
    } else {
        std::wifstream file(file_path.c_str());

        if(file.is_open()){
            if(file.good()){
                file >> data.next_id;
                file.get();

                std::wstring line;
                while(file.good() && getline(file, line)){
                    std::vector<std::wstring> parts;
                    boost::split(parts, line, boost::is_any_of(L":"), boost::token_compress_on);

                    T entry;

                    parts >> entry;

                    data.data.push_back(std::move(entry));
                }
            }
        }
    }
}

template<typename T>
bool exists(const data_handler<T>& data, std::size_t id){
    for(auto& entry : data.data){
        if(entry.id == id){
            return true;
        }
    }

    return false;
}

template<typename T>
void remove(data_handler<T>& data, std::size_t id){
    data.data.erase(std::remove_if(data.data.begin(), data.data.end(),
        [id](const T& entry){ return entry.id == id; }), data.data.end());
}

template<typename T>
void add_data(data_handler<T>& data, T&& entry){
    entry.id = data.next_id++;

    data.data.push_back(std::forward<T>(entry));
}

} //end of namespace budget

#endif
