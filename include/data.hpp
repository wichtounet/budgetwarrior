//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "cpp_utils/assert.hpp"

#include "config.hpp"
#include "logging.hpp"
#include "utils.hpp"
#include "api.hpp"
#include "server_lock.hpp"
#include "budget_exception.hpp"

namespace budget {

struct date;
struct money;

struct data_reader {
    void parse(const std::string& data);

    data_reader& operator>>(bool& value);
    data_reader& operator>>(size_t& value);
    data_reader& operator>>(int64_t& value);
    data_reader& operator>>(int32_t& value);
    data_reader& operator>>(double& value);
    data_reader& operator>>(std::string& value);
    data_reader& operator>>(budget::date& value);
    data_reader& operator>>(budget::money& value);

    bool more() const;
    void skip();
    std::string peek() const;

private:
    std::vector<std::string> parts;
    size_t                   current = 0;
};

struct data_writer {
    data_writer& operator<<(const bool& value);
    data_writer& operator<<(const size_t& value);
    data_writer& operator<<(const int64_t& value);
    data_writer& operator<<(const int32_t& value);
    data_writer& operator<<(const std::string& value);
    data_writer& operator<<(const budget::date& value);
    data_writer& operator<<(const budget::money& value);

    std::string to_string() const;

private:
    std::vector<std::string> parts;
};

template<typename T>
struct data_handler {
    size_t next_id{};  // Note: No need to protect this since this is only accessed by GC (not run from server)

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
        server_lock_guard l(lock);

        set_changed_internal();
    }

    template<typename Functor>
    void parse_stream(std::istream& file, Functor f){
        next_id = 1;

        std::string line;
        while (file.good() && getline(file, line)) {
            if (line.empty()) {
                continue;
            }

            data_reader reader;
            reader.parse(line);

            T entry;

            f(reader, entry);

            if (entry.id >= next_id) {
                next_id = entry.id + 1;
            }

            data_.push_back(std::move(entry));
        }
    }

    template<typename Functor>
    void load(Functor f){
        //Make sure to clear the data first, as load_data can be called
        //several times
        data_.clear();

        if(is_server_mode()){
            auto res = budget::api_get(std::string("/") + module + "/list/");

            if(res.success){
                std::stringstream ss(res.result);
                parse_stream(ss, f);
            }
        } else {
            auto file_path = path_to_budget_file(path);

            if (!std::filesystem::exists(file_path)) {
                next_id = 1;
            } else {
                std::ifstream file(file_path);

                if (file.is_open()) {
                    if (file.good()) {
                        // We do not use the next_id saved anymore
                        // Simply consume the line
                        std::string id_line;
                        getline(file, id_line);

                        parse_stream(file, f);
                    }
                }
            }
        }
    }

    void load(){
        load([](data_reader& reader, T& entry){ entry.load(reader); });
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

    bool indirect_edit(const T& value, bool propagate = true) {
        server_lock_guard l(lock);

        if (is_server_mode()) {
            auto params = value.get_params();

            auto res = budget::api_post(std::string("/") + get_module() + "/edit/", params);

            if (!res.success) {
                LOG_F(ERROR, "Failed to edit from {}", get_module());

                return false;
            }
            return true;

        }
        for (auto& v : data_) {
            if (v.id == value.id) {
                v = value;

                if (propagate) {
                    set_changed_internal();
                }

                return true;
            }
        }

        return false;
    }

    template <typename TT>
    size_t add(TT&& entry) {
        server_lock_guard l(lock);

        if (is_server_mode()) {
            auto params = entry.get_params();

            auto res = budget::api_post(std::string("/") + get_module() + "/add/", params);

            if (!res.success) {
                LOG_F(ERROR, "Failed to add data from module {}", get_module());

                entry.id = 0;
            } else {
                entry.id = budget::to_number<size_t>(res.result);

                data_.emplace_back(std::forward<TT>(entry));
            }
        } else {
            entry.id = next_id++;

            data_.emplace_back(std::forward<TT>(entry));

            set_changed_internal();
        }

        return entry.id;
    }

    bool remove(size_t id) {
        server_lock_guard l(lock);

        auto before = data_.size();
        data_.erase(std::remove_if(data_.begin(), data_.end(),
                                  [id](const T& entry) { return entry.id == id; }),
                   data_.end());

        if (is_server_mode()) {
            auto res = budget::api_get(std::string("/") + get_module() + "/delete/?input_id=" + budget::to_string(id));

            if (!res.success) {
                LOG_F(ERROR, "Failed to delete data from module {}", get_module());
            }

            return res.success;
        }
        set_changed_internal();

        return data_.size() < before;
    }

    bool exists(size_t id) {
        server_lock_guard l(lock);

        for (auto& entry : data_) {
            if (entry.id == id) {
                return true;
            }
        }

        return false;
    }

    T operator[](size_t id) const {
        server_lock_guard l(lock);

        for (auto& value : data_) {
            if (value.id == id) {
                return value;
            }
        }

        throw budget_exception("There is no data with id " + std::to_string(id) + " in " + module);
    }

    size_t size() const {
        server_lock_guard l(lock);
        return data_.size();
    }

    bool empty() const {
        server_lock_guard l(lock);
        return data_.empty();
    }

    const char* get_module() const {
        return module;
    }

    std::vector<T> data() const {
        std::vector<T> copy;
        copy.reserve(size());

        {
            server_lock_guard l(lock);
            std::copy(data_.begin(), data_.end(), std::back_inserter(copy));
        }

        return copy;
    }

    // This can only be accessed during loading
    std::vector<T> & unsafe_data() {
        return data_;
    }

private:
    void set_changed_internal() {
        if (is_server_running()) {
            force_save();
        } else {
            changed = true;
        }
    }

    void force_save() {
        cpp_assert(!is_server_mode(), "force_save() should never be called in server mode");

        if (budget::config_contains("random")) {
            LOG_F(ERROR, "Saving is disabled in random mode");
            return;
        }

        auto file_path = path_to_budget_file(path);

        std::ofstream file(file_path);

        // We still save the file ID so that it's still compatible with older versions for now
        file << next_id << std::endl;

        for (auto& entry : data_) {
            data_writer writer;
            entry.save(writer);
            file << writer.to_string() << std::endl;
        }

        changed = false;
    }

    const char* module;
    const char* path;
    volatile bool changed = false;
    mutable server_lock lock;
    std::vector<T> data_;
};

bool migrate_database(size_t old_data_version);

} //end of namespace budget
