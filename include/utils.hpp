//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace budget {

std::wstring to_wstring(const std::string& source);
std::string to_nstring(const std::wstring& source);

/*!
 * \brief Convert a string to a number of an arbitrary type.
 * \param text The string to convert.
 * \return The converted text in the good type.
 */
template <typename T>
inline T to_number (const std::wstring& text) {
    std::wstringstream ss(text);
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
inline int to_number (const std::wstring& text) {
    return boost::lexical_cast<int>(text);
}

template<typename T>
inline std::wstring to_string(T value){
    return to_wstring(std::to_string(value));
}

template<>
inline std::wstring to_string(boost::gregorian::date date){
    std::string string_date = boost::gregorian::to_iso_extended_string(date);
    return to_wstring(string_date);
}

} //end of namespace budget

#endif
