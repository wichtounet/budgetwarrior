//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <fstream>
#include <cstdint>

#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif
#include <sys/stat.h>

#include "cpp_utils/assert.hpp"

#include "utils.hpp"
#include "budget_exception.hpp"
#include "config.hpp"
#include "expenses.hpp"
#include "earnings.hpp"

unsigned short budget::terminal_width(){
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    SHORT columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return static_cast<unsigned short>(columns);
#else
    struct winsize w {};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}

unsigned short budget::terminal_height(){
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    SHORT rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return static_cast<unsigned short>(rows);
#else
    struct winsize w {};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
#endif
}

std::vector<std::string>& budget::split(std::string_view s, char delim, std::vector<std::string>& elems) {
    for (auto element : s | std::ranges::views::split(delim)) {
#ifdef __clang__
        elems.emplace_back(std::string_view(&*element.begin(), std::ranges::distance(element)));
#else
        elems.emplace_back(std::string_view(element));
#endif
    }
    return elems;
}

std::vector<std::string> budget::split(std::string_view s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::vector<std::string_view>& budget::splitv(std::string_view s, char delim, std::vector<std::string_view>& elems) {
    for (auto element : s | std::ranges::views::split(delim)) {
#ifdef __clang__
        elems.emplace_back(&*element.begin(), std::ranges::distance(element));
#else
        elems.emplace_back(element);
#endif
    }
    return elems;
}

std::vector<std::string_view> budget::splitv(std::string_view s, char delim) {
    std::vector<std::string_view> elems;
    splitv(s, delim, elems);
    return elems;
}

std::string budget::base64_decode(std::string_view in) {
    // table from '+' to 'z'
    const uint8_t lookup[] = {
        62, 255, 62, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
        255, 0, 255, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
        255, 255, 255, 255, 63, 255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

    std::string out;
    int val = 0;
    int valb = -8;
    for (uint8_t c : in) {
        if (c < '+' || c > 'z') {
            break;
        }

        c -= '+';

        if (lookup[c] >= 64){
            break;
        }

        val = (val << 6) + lookup[c];
        valb += 6;

        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return out;
}

std::string budget::base64_encode(std::string_view in){
    std::string out;

    std::string lookup = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int val =0;
    int valb = -6;

    for (const char c : in) {
        val = (val << 8) + c;
        valb += 8;

        while(valb >= 0){
            out.push_back(lookup[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    if(valb > -6){
        out.push_back(lookup[((val << 8) >> (valb + 8))&0x3F]);
    }

    while(out.size() % 4){
        out.push_back('=');
    }

    return out;
}

std::string budget::html_base64_decode(std::string_view in) {
    std::string out(in);

    std::replace(out.begin(), out.end(), '_', '=');

    return base64_decode(out);
}

std::string budget::html_base64_encode(std::string_view in){
    std::string out = base64_encode(in);

    std::replace(out.begin(), out.end(), '=', '_');

    return out;
}
