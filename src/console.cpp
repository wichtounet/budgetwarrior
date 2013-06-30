//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <sstream>

#include <boost/algorithm/string.hpp>

#include "console.hpp"

std::string budget::format_code(int attr, int fg, int bg){
    std::stringstream stream;
    stream << "" << '\033' << "[" << attr << ";" << (fg + 30) << (bg + 40) << "m";
    return stream.str();
}

void budget::display_table(std::vector<std::string> columns, std::vector<std::vector<std::string>> contents){
    std::vector<std::size_t> widths;

    for(auto& row : contents){
        for(auto& cell : row){
            boost::algorithm::trim(cell);
        }
    }

    for(auto& column : columns){
        widths.push_back(column.length());
    }

    for(auto& row : contents){
        for(std::size_t i = 0; i < columns.size(); ++i){
            widths[i] = std::max(widths[i], row[i].size());
        }
    }

    for(std::size_t i = 0; i < columns.size(); ++i){
        auto& column = columns[i];

        std::cout << format_code(4, 0, 7) << column << std::string(widths[i] - column.size(), ' ') << format_code(0, 0, 7);

        if(i < columns.size() - 1){
            std::cout << " ";
        }
    }

    std::cout << std::endl;

    for(std::size_t i = 0; i < contents.size(); ++i){
        auto& row = contents[i];

        if(i % 2 == 0){
            std::cout << format_code(7, 0, 7);
        } else {
            std::cout << format_code(0, 0, 7);
        }

        for(std::size_t j = 0; j < columns.size(); ++j){
            std::cout << row[j] << std::string(widths[j] + (j < columns.size() - 1 ? 1 : 0) - row[j].size(), ' ');
        }

        std::cout << format_code(0, 0, 7) << std::endl;
    }
}
