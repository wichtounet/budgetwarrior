//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <vector>

namespace budget {

/*!
 * \brief Convert a string to a number of an arbitrary type.
 * \param text The string to convert.
 * \return The converted text in the good type.
 */
template <typename T>
inline T to_number (const std::string& text) {
    std::stringstream ss(text);
    T result;
    ss >> result;
    return result;
}

/*!
 * \brief Convert a string to a number of an int. This function is optimized for speed when converting int.
 * \param text The string to convert.
 * \return The text converted to an int.
 */
template<>
inline int to_number (const std::string& text) {
    return std::stoi(text);
}

template<>
inline unsigned short to_number (const std::string& text) {
    return static_cast<unsigned short>(std::stoi(text));
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

void one_of(const std::string& value, const std::string& message, std::vector<std::string> values);

unsigned short terminal_width();
unsigned short terminal_height();

} //end of namespace budget

#endif
