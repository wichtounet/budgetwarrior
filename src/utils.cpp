//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <cstdio>
#include <fstream>

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
    struct winsize w;
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
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
#endif
}

bool budget::file_exists(const std::string& name){
    std::ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }
}

bool budget::folder_exists(const std::string& name){
    struct stat sb;
    return stat(name.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode);
}

std::vector<std::string>& budget::split(const std::string& s, char delim, std::vector<std::string>& elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> budget::split(const std::string& s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::string budget::base64_decode(const std::string& in) {
    // table from '+' to 'z'
    const uint8_t lookup[] = {
        62, 255, 62, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
        255, 0, 255, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
        255, 255, 255, 255, 63, 255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

    std::string out;
    int val = 0, valb = -8;
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

std::string budget::base64_encode(const std::string& in){
    std::string out;

    std::string lookup = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int val =0;
    int valb = -6;

    for(char c : in){
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
