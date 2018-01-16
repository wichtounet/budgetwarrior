//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "cpp_utils/assert.hpp"

#include "config.hpp"
#include "utils.hpp"
#include "server.hpp"
#include "api.hpp"

namespace budget {

template<typename T>
struct data_handler {
    size_t next_id;
    std::vector<T> data;

    data_handler(const char* module, const char* path) : module(module), path(path) {
        // Nothing else to init
    };

    //data_handler should never be copied
    data_handler(const data_handler& rhs) = delete;
    data_handler& operator=(const data_handler& rhs) = delete;

    bool is_changed() const {
        return changed;
    }

    void set_changed() {
        if (is_server_running()) {
            force_save();
        } else {
            changed = true;
        }
    }

    template<typename Functor>
    void parse_stream(std::istream& file, Functor f){
        next_id = 1;

        std::string line;
        while (file.good() && getline(file, line)) {
            if (line.empty()) {
                continue;
            }

            auto parts = split(line, ':');

            T entry;

            f(parts, entry);

            if (entry.id >= next_id) {
                next_id = entry.id + 1;
            }

            data.push_back(std::move(entry));
        }
    }

    template<typename Functor>
    void load(Functor f){
        //Make sure to clear the data first, as load_data can be called
        //several times
        data.clear();

        if(is_server_mode()){
            auto res = budget::api_get(std::string("/") + module + "/list/");

            if(res.success){
                std::stringstream ss(res.result);
                parse_stream(ss, f);
            }
        } else {
            auto file_path = path_to_budget_file(path);

            if (!file_exists(file_path)) {
                next_id = 1;
            } else {
                std::ifstream file(file_path);

                if (file.is_open()) {
                    if (file.good()) {
                        // We do not use the next_id saved anymore
                        // Simply consume it
                        size_t fake;
                        file >> fake;
                        file.get();

                        parse_stream(file, f);
                    }
                }
            }
        }
    }

    void load(){
        load([](std::vector<std::string>& parts, T& entry){ parts >> entry; });
    }

    void force_save() {
        cpp_assert(!is_server_mode(), "force_save() should never be called in server mode");

        if (budget::config_contains("random")) {
            std::cerr << "budget: error: Saving is disabled in random mode" << std::endl;
            return;
        }

        auto file_path = path_to_budget_file(path);

        std::ofstream file(file_path);

        // We still save the file ID so that it's still compatible with older versions for now
        file << next_id << std::endl;

        for (auto& entry : data) {
            file << entry << std::endl;
        }

        changed = false;
    }

    void save() {
        // In server mode, there is nothing to do
        if (is_server_mode()) {
            // It shoud not be changed
            cpp_assert(!is_changed(), "in server mode, is_changed() should never be true");

            return;
        }

        // In other modes, save if it's changed
        if (is_changed()) {
            force_save();
        }
    }

    bool edit(T& value){
        if(is_server_mode()){
            auto params = value.get_params();

            auto res = budget::api_post(std::string("/") + get_module() + "/edit/", params);

            if (!res.success) {
                std::cerr << "error: Failed to edit from " << get_module() << std::endl;

                return false;
            } else {
                return true;
            }
        } else {
            set_changed();

            return true;
        }
    }

    size_t add(T&& entry) {
        if (is_server_mode()) {
            auto params = entry.get_params();

            auto res = budget::api_post(std::string("/") + get_module() + "/add/", params);

            if (!res.success) {
                std::cerr << "error: Failed to add expense" << std::endl;

                entry.id = 0;
            } else {
                entry.id = budget::to_number<size_t>(res.result);

                data.push_back(std::forward<T>(entry));
            }
        } else {
            entry.id = next_id++;

            data.push_back(std::forward<T>(entry));

            set_changed();
        }

        return entry.id;
    }

    void remove(size_t id) {
        data.erase(std::remove_if(data.begin(), data.end(),
                                  [id](const T& entry) { return entry.id == id; }),
                   data.end());

        if (is_server_mode()) {
            std::map<std::string, std::string> params;

            params["input_id"] = budget::to_string(id);

            auto res = budget::api_post(std::string("/") + get_module() + "/delete/", params);

            if (!res.success) {
                std::cerr << "error: Failed to delete from " << get_module() << std::endl;
            }
        } else {
            set_changed();
        }
    }

    bool exists(size_t id) {
        for (auto& entry : data) {
            if (entry.id == id) {
                return true;
            }
        }

        return false;
    }

    T& operator[](size_t id) {
        for (auto& value : data) {
            if (value.id == id) {
                return value;
            }
        }

        cpp_unreachable("The data must exists");
    }

    size_t size() const {
        return data.size();
    }

    decltype(auto) begin() {
        return data.begin();
    }

    decltype(auto) begin() const {
        return data.begin();
    }

    decltype(auto) end() {
        return data.end();
    }

    decltype(auto) end() const {
        return data.end();
    }

    const char* get_module() const {
        return module;
    }

private:
    const char* module;
    const char* path;
    bool changed = false;
};

} //end of namespace budget
