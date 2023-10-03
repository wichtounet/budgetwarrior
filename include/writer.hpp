//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "data_cache.hpp"
#include "date.hpp"
#include "money.hpp"

namespace budget {

struct end_of_line_t {};
struct title_begin_t {};
struct title_end_t {};
struct p_begin_t {};
struct p_end_t {};

static constexpr end_of_line_t end_of_line;
static constexpr p_begin_t p_begin;
static constexpr p_end_t p_end;
static constexpr title_begin_t title_begin;
static constexpr title_end_t title_end;

struct year_month_selector {
    std::string page;
    budget::year current_year;
    budget::month current_month;

    year_month_selector(std::string page, budget::year current_year, budget::month current_month)
        : page(std::move(std::move(page))), current_year(current_year), current_month(current_month) {}
};

struct year_selector {
    std::string page;
    budget::year current_year;

    year_selector(std::string page, budget::year current_year)
        : page(std::move(std::move(page))), current_year(current_year) {}
};

struct asset_selector {
    std::string page;
    std::size_t current_asset;

    asset_selector(std::string page, std::size_t current_asset)
        : page(std::move(std::move(page))), current_asset(current_asset) {}
};

struct active_asset_selector {
    std::string page;
    std::size_t current_asset;

    active_asset_selector(std::string page, std::size_t current_asset)
        : page(std::move(std::move(page))), current_asset(current_asset) {}
};

struct add_button {
    std::string module;

    explicit add_button(std::string module) : module(std::move(std::move(module))) {}
};

struct set_button {
    std::string module;

    explicit set_button(std::string module) : module(std::move(std::move(module))) {}
};

struct writer {
    writer()              = default;
    writer(const writer&) = delete;
    writer(writer&&)      = delete;
    writer& operator=(const writer&) = delete;
    writer& operator=(writer&&) = delete;
    virtual ~writer()           = default;

    data_cache cache;

    virtual writer& operator<<(std::string_view value) = 0;
    virtual writer& operator<<(double value) = 0;

    virtual writer& operator<<(const budget::money& m) = 0;
    virtual writer& operator<<(const budget::month& m) = 0;
    virtual writer& operator<<(const budget::year& m) = 0;

    virtual writer& operator<<(const end_of_line_t& m) = 0;
    virtual writer& operator<<(const p_begin_t& m) = 0;
    virtual writer& operator<<(const p_end_t& m) = 0;
    virtual writer& operator<<(const title_begin_t& m) = 0;
    virtual writer& operator<<(const title_end_t& m) = 0;

    virtual bool is_web() = 0;

    virtual writer& operator<<(const year_month_selector&){
        return *this;
    }

    virtual writer& operator<<(const year_selector&){
        return *this;
    }

    virtual writer& operator<<(const asset_selector&){
        return *this;
    }

    virtual writer& operator<<(const active_asset_selector&){
        return *this;
    }

    virtual writer& operator<<(const add_button&){
        return *this;
    }

    virtual writer& operator<<(const set_button&){
        return *this;
    }

    virtual void display_table(std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents, size_t groups = 1, std::vector<size_t> lines = {}, size_t left = 0, size_t foot = 0) = 0;
    virtual void display_graph(std::string_view title, std::vector<std::string>& categories, std::vector<std::string> series_names, std::vector<std::vector<float>>& series_values) = 0;
};

struct console_writer : writer {
    std::ostream& os;

    explicit console_writer(std::ostream& os);

    console_writer(const console_writer&) = delete;
    console_writer(console_writer&&)      = delete;
    console_writer& operator=(const console_writer&) = delete;
    console_writer& operator=(console_writer&&) = delete;
    ~console_writer() override                  = default;

    writer& operator<<(std::string_view value) override;
    writer& operator<<(double value) override;

    writer& operator<<(const budget::money& m) override;
    writer& operator<<(const budget::month& m) override;
    writer& operator<<(const budget::year& m) override;

    writer& operator<<(const end_of_line_t& m) override;
    writer& operator<<(const p_begin_t& m) override;
    writer& operator<<(const p_end_t& m) override;
    writer& operator<<(const title_begin_t& m) override;
    writer& operator<<(const title_end_t& m) override;

    bool is_web() override;

    void display_table(std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents,
                       size_t groups = 1, std::vector<size_t> lines = {}, size_t left = 0, size_t foot = 0) override;
    void display_graph(std::string_view title, std::vector<std::string>& categories,
                       std::vector<std::string> series_names, std::vector<std::vector<float>>& series_values) override;
};

} //end of namespace budget
