//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <charconv>

#include "data.hpp"
#include "utils.hpp"
#include "date.hpp"
#include "money.hpp"

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
        while ((start_pos = part.find(":", start_pos)) != std::string::npos) {
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
    str.erase(std::remove(str.begin(), str.end(), ','), str.end());
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
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

    size_t temp;
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
    value = budget::parse_money(parts.at(current));
    ++current;
    return *this;
}

bool budget::data_reader::more() const {
    return current < parts.size();
}

// data_writer

budget::data_writer& budget::data_writer::operator<<(const bool& value){
    parts.emplace_back(std::to_string(static_cast<size_t>(value)));
    return *this;
}

budget::data_writer& budget::data_writer::operator<<(const size_t& value){
    parts.emplace_back(std::to_string(value));
    return *this;
}

budget::data_writer& budget::data_writer::operator<<(const int64_t& value){
    parts.emplace_back(std::to_string(value));
    return *this;
}

budget::data_writer& budget::data_writer::operator<<(const int32_t& value){
    parts.emplace_back(std::to_string(value));
    return *this;
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
