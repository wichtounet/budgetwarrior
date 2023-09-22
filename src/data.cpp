//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <charconv>
#include <array>

#include "data.hpp"
#include "utils.hpp"
#include "date.hpp"
#include "money.hpp"

// For database migration
#include "assets.hpp"
#include "liabilities.hpp"

namespace {

void parse_input(std::vector<std::string>& parts) {
    for (auto& part : parts) {
        size_t start_pos = 0;
        while ((start_pos = part.find("\\x3A", start_pos)) != std::string::npos) {
            part.replace(start_pos, 4, ":");
            ++start_pos;
        }
    }
}

std::string parse_output(const std::vector<std::string>& parts) {
    std::string output;
    std::string sep;

    for (auto part : parts) {
        size_t start_pos = 0;
        while ((start_pos = part.find(':', start_pos)) != std::string::npos) {
            part.replace(start_pos, 1, "\\x3A");
            start_pos += 4;
        }

        output += sep;
        output += part;
        sep = ":";
    }

    return output;
}

// Note: This function is necessary because writing numbers used to be
// locale-dependent. To read older database, we need to handle , in numbers
// and spaces as practical utility
std::string pre_clean_number(std::string str) {
    std::erase(str, ',');
    std::erase(str, ' ');
    return str;
}

} // namespace

// data_reader

void budget::data_reader::parse(const std::string& data) {
    parts   = split(data, ':');
    current = 0;

    parse_input(parts);
}

budget::data_reader& budget::data_reader::operator>>(bool& value) {
    auto part = pre_clean_number(parts.at(current));

    size_t temp = 0;
    if (auto [p, ec] = std::from_chars(part.data(), part.data() + part.size(), temp); ec != std::errc() || p != part.data() + part.size()) {
        throw budget::budget_exception("\"" + parts.at(current) + "\" is not a valid bool");
    }

    value = temp;

    ++current;
    return *this;
}

budget::data_reader& budget::data_reader::operator>>(size_t& value) {
    auto part = pre_clean_number(parts.at(current));

    if (auto [p, ec] = std::from_chars(part.data(), part.data() + part.size(), value); ec != std::errc() || p != part.data() + part.size()) {
        throw budget::budget_exception("\"" + parts.at(current) + "\" is not a valid size_t");
    }

    ++current;
    return *this;
}

budget::data_reader& budget::data_reader::operator>>(int64_t& value) {
    auto part = pre_clean_number(parts.at(current));

    if (auto [p, ec] = std::from_chars(part.data(), part.data() + part.size(), value); ec != std::errc() || p != part.data() + part.size()) {
        throw budget::budget_exception("\"" + parts.at(current) + "\" is not a valid int64_t");
    }

    ++current;
    return *this;
}

budget::data_reader& budget::data_reader::operator>>(int32_t& value) {
    auto part = pre_clean_number(parts.at(current));

    if (auto [p, ec] = std::from_chars(part.data(), part.data() + part.size(), value); ec != std::errc() || p != part.data() + part.size()) {
        throw budget::budget_exception("\"" + parts.at(current) + "\" is not a valid int32_t");
    }

    ++current;
    return *this;
}

budget::data_reader& budget::data_reader::operator>>(double& value) {
    auto part = pre_clean_number(parts.at(current));

    if (auto [p, ec] = std::from_chars(part.data(), part.data() + part.size(), value); ec != std::errc() || p != part.data() + part.size()) {
        throw budget::budget_exception("\"" + parts.at(current) + "\" is not a valid double");
    }

    ++current;
    return *this;
}

budget::data_reader& budget::data_reader::operator>>(std::string& value) {
    value = parts.at(current);
    ++current;
    return *this;
}

budget::data_reader& budget::data_reader::operator>>(budget::date& value) {
    value = budget::date_from_string(parts.at(current));
    ++current;
    return *this;
}

budget::data_reader& budget::data_reader::operator>>(budget::money& value) {
    value = budget::money_from_string(parts.at(current));
    ++current;
    return *this;
}

bool budget::data_reader::more() const {
    return current < parts.size();
}

std::string budget::data_reader::peek() const {
    return parts.at(current);
}

void budget::data_reader::skip() {
    if (more()) {
        ++current;
    }
}

// data_writer

budget::data_writer& budget::data_writer::operator<<(const bool& value){
    const size_t temp = value;

    std::array<char, 64> buffer{};

    if (auto [p, ec] = std::to_chars(buffer.begin(), buffer.end(), temp); ec == std::errc()) {
        parts.emplace_back(buffer.begin(), p);
        return *this;
    }
    throw budget::budget_exception("\"" + std::to_string(temp) + "\" cant' be converted to string");
}

budget::data_writer& budget::data_writer::operator<<(const size_t& value){
    std::array<char, 64> buffer{};

    if (auto [p, ec] = std::to_chars(buffer.begin(), buffer.end(), value); ec == std::errc()) {
        parts.emplace_back(buffer.begin(), p);
        return *this;
    }
    throw budget::budget_exception("\"" + std::to_string(value) + "\" cant' be converted to string");
}

budget::data_writer& budget::data_writer::operator<<(const int64_t& value){
    std::array<char, 64> buffer{};

    if (auto [p, ec] = std::to_chars(buffer.begin(), buffer.end(), value); ec == std::errc()) {
        parts.emplace_back(buffer.begin(), p);
        return *this;
    }
    throw budget::budget_exception("\"" + std::to_string(value) + "\" cant' be converted to string");
}

budget::data_writer& budget::data_writer::operator<<(const int32_t& value){
    std::array<char, 64> buffer{};

    if (auto [p, ec] = std::to_chars(buffer.begin(), buffer.end(), value); ec == std::errc()) {
        parts.emplace_back(buffer.begin(), p);
        return *this;
    }
    throw budget::budget_exception("\"" + std::to_string(value) + "\" cant' be converted to string");
}

budget::data_writer& budget::data_writer::operator<<(const std::string& value){
    parts.emplace_back(value);
    return *this;
}

budget::data_writer& budget::data_writer::operator<<(const budget::date& value){
    parts.emplace_back(budget::date_to_string(value));
    return *this;
}

budget::data_writer& budget::data_writer::operator<<(const budget::money& value){
    parts.emplace_back(budget::to_string(value));
    return *this;
}

std::string budget::data_writer::to_string() const {
    return parse_output(parts);
}

bool budget::migrate_database(size_t old_data_version) {
    if (old_data_version > DATA_VERSION) {
        LOG_F(ERROR, "Unsupported database version ({}), you should update budgetwarrior", old_data_version);

        return false;
    }

    if (old_data_version < MIN_DATA_VERSION) {
        LOG_F(ERROR, "Your database version ({}) is not supported anymore", old_data_version);
        LOG_F(ERROR, "You can use an older version of budgetwarrior to migrate it");

        return false;
    }

    if (old_data_version < DATA_VERSION) {
        LOG_F(INFO, "Migrating database to version {}...", DATA_VERSION);

        if (old_data_version <= 4 && DATA_VERSION >= 5) {
            migrate_assets_4_to_5();
        }

        if (old_data_version <= 5 && DATA_VERSION >= 6) {
            migrate_assets_5_to_6();
        }

        if (old_data_version <= 6 && DATA_VERSION >= 7) {
            migrate_liabilities_6_to_7();
        }

        if (old_data_version <= 7 && DATA_VERSION >= 8) {
            migrate_assets_7_to_8();
        }

        if (old_data_version <= 8 && DATA_VERSION >= 9) {
            migrate_assets_8_to_9();
        }

        internal_config_set("data_version", to_string(DATA_VERSION));

        // We want to make sure the new data version is set in stone!
        save_config();

        LOG_F(INFO, "Migrated to database version {}...", DATA_VERSION);
    }

    return true;
}
