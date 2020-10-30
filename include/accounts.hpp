//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <vector>
#include <string>
#include <map>

#include "module_traits.hpp"
#include "money.hpp"
#include "date.hpp"
#include "writer_fwd.hpp"

namespace budget {

struct data_reader;
struct data_writer;

struct accounts_module {
    void load();
    void unload();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<accounts_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "account";
};

struct account {
    size_t id;
    std::string guid;
    std::string name;
    money amount;
    date since;
    date until;

    std::map<std::string, std::string> get_params() const ;

    void load(data_reader & reader);
    void save(data_writer & writer);
};

void load_accounts();
void save_accounts();

bool account_exists(const std::string& account);

std::vector<std::string> all_account_names();

struct data_cache;

std::vector<budget::account> all_accounts();
std::vector<budget::account> all_accounts(data_cache & cache, year year, month month);
std::vector<budget::account> current_accounts();

budget::account get_account(size_t id);
budget::account get_account(std::string name, year year, month month);

void set_accounts_changed();
void set_accounts_next_id(size_t next_id);

void show_all_accounts(budget::writer& w);
void show_accounts(budget::writer& w);

void add_account(account&& account);
bool account_exists(size_t id);
void account_delete(size_t id);

date find_new_since();

void archive_accounts_impl(bool month);

} //end of namespace budget
