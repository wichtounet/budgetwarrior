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

void budget::display_table(std::vector<std::string> columns, std::vector<std::vector<std::string>> contents, std::size_t groups){
    assert(groups > 0);

    for(auto& row : contents){
        for(auto& cell : row){
            boost::algorithm::trim(cell);
        }
    }

    std::vector<std::size_t> widths;
    std::vector<std::size_t> header_widths;

    if(!contents.size()){
        for(auto& column : columns){
            widths.push_back(column.length());
        }
    } else {
        auto& first = contents[0];

        for(auto& value : first){
            widths.push_back(0);
        }

        for(auto& row : contents){
            for(std::size_t i = 0; i < widths.size(); ++i){
                widths[i] = std::max(widths[i], row[i].length() + (i < row.size() - 1 ? 1 : 0));
            }
        }
    }

    assert(widths.size() == groups * columns.size());

    for(std::size_t i = 0; i < columns.size(); ++i){
        auto& column = columns[i];

        std::size_t width = 0;
        for(std::size_t j = i * groups; j < (i + 1) * groups; ++j){
            width += widths[j];
        }

        width = std::max(width, column.length());
        header_widths.push_back(width + (i < columns.size() - 1 ? 1 : 0));

        std::cout << format_code(4, 0, 7) << column << std::string(width - column.length(), ' ') << format_code(0, 0, 7);

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

        for(std::size_t j = 0; j < row.size(); ++j){
            auto width = widths[j];
            if(j % groups == 0){
                width = std::max(width, header_widths[j / groups]);
            }

            std::cout << row[j] << std::string(width - row[j].length(), ' ');
        }

        std::cout << format_code(0, 0, 7) << std::endl;
    }
}
