//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef DATA_H
#define DATA_H

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "config.hpp"
#include "assert.hpp"

namespace budget {

template<typename T>
struct data_handler {
    std::size_t next_id;
    std::vector<T> data;
    bool changed = false;

    data_handler(){};

    //data_handler should never be copied
    data_handler(const data_handler& rhs) = delete;
    data_handler& operator=(const data_handler& rhs) = delete;
};

template<typename T>
void save_data(const data_handler<T>& data, const std::string& path){
    if(data.changed){
        auto file_path = path_to_budget_file(path);

        std::ofstream file(file_path);
        file << data.next_id << std::endl;

        for(auto& entry: data.data){
            file << entry << std::endl;
        }
    }
}

template<typename T, typename Functor>
void load_data(data_handler<T>& data, const std::string& path, Functor f){
    auto file_path = path_to_budget_file(path);

    if(!boost::filesystem::exists(file_path)){
        data.next_id = 1;
    } else {
        std::ifstream file(file_path);

        if(file.is_open()){
            if(file.good()){
                //Make sure to clear the data first, as load_data can be called
                //several times
                data.data.clear();

                file >> data.next_id;
                file.get();

                std::string line;
                while(file.good() && getline(file, line)){
                    std::vector<std::string> parts;
                    boost::split(parts, line, boost::is_any_of(":"), boost::token_compress_on);

                    T entry;

                    f(parts, entry);

                    data.data.push_back(std::move(entry));
                }
            }
        }
    }
}

template<typename T>
void load_data(data_handler<T>& data, const std::string& path){
    load_data(data, path, [](std::vector<std::string>& parts, T& entry){ parts >> entry; });
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

    data.changed = true;
}

template<typename T>
T& get(data_handler<T>& data, std::size_t id){
    for(auto& value : data.data){
        if(value.id == id){
            return value;
        }
    }

    budget_unreachable("The data must exists");
}

template<typename T>
std::size_t add_data(data_handler<T>& data, T&& entry){
    entry.id = data.next_id++;

    data.data.push_back(std::forward<T>(entry));

    data.changed = true;

    return entry.id;
}

} //end of namespace budget

#endif
