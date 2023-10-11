//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <iomanip>
#include <charconv>

#include "budget_exception.hpp"

namespace budget {

/*!
 * \brief Convert a string to a number of an arbitrary type.
 * \param text The string to convert.
 * \return The converted text in the good type.
 */
template <typename T>
inline T to_number(std::string_view text) {
    T result;
    if (auto [p, ec] = std::from_chars(text.data(), text.data() + text.size(), result); ec == std::errc()) {
        return result;
    }
    throw budget::budget_exception("\"" + std::string(text) + "\" is not a valid number");
}

template<typename T>
inline std::string to_string(T value){
    return std::to_string(value);
}

template<>
inline std::string to_string(std::string value){
    return value;
}

template<>
inline std::string to_string(const char* value){
    return value;
}

template<typename T>
inline std::string to_string_precision(T value, int precision){
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

void one_of(const std::string& value, const std::string& message, std::vector<std::string> values);

unsigned short terminal_width();
unsigned short terminal_height();

std::vector<std::string> split(std::string_view s, char delim);
std::vector<std::string> &split(std::string_view s, char delim, std::vector<std::string> &elems);

std::vector<std::string_view> splitv(std::string_view s, char delim);
std::vector<std::string_view> &splitv(std::string_view s, char delim, std::vector<std::string_view> &elems);

std::string base64_decode(std::string_view in);
std::string base64_encode(std::string_view in);

std::string html_base64_decode(std::string_view in);
std::string html_base64_encode(std::string_view in);

std::string exec_command(const std::string& command);

} //end of namespace budget
