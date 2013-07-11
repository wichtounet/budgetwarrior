//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <sstream>

#include <boost/algorithm/string.hpp>

#include "console.hpp"

std::wstring budget::format_code(int attr, int fg, int bg){
    std::wstringstream stream;
    stream << "" << '\033' << "[" << attr << ";" << (fg + 30) << (bg + 40) << "m";
    return stream.str();
}

void budget::display_table(std::vector<std::wstring> columns, std::vector<std::vector<std::wstring>> contents, std::size_t groups){
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
            for(std::size_t i = 0; i < row.size(); ++i){
                widths[i] = std::max(widths[i], row[i].length() + 1);
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
        header_widths.push_back(width + (i < columns.size() - 1 && column.length() >= width ? 1 : 0));

        //The last space is not underlined
        --width;

        std::wcout << format_code(4, 0, 7) << column << (width > column.length() ? std::wstring(width - column.length(), ' ') : L"") << format_code(0, 0, 7);

        //The very last column has no trailing space

        if(i < columns.size() - 1){
            std::wcout << " ";
        }
    }

    std::wcout << std::endl;

    for(std::size_t i = 0; i < contents.size(); ++i){
        auto& row = contents[i];

        if(i % 2 == 0){
            std::wcout << format_code(7, 0, 7);
        } else {
            std::wcout << format_code(0, 0, 7);
        }


        for(std::size_t j = 0; j < row.size(); j += groups){
            std::size_t acc_width = 0;

            //First columns of the group
            for(std::size_t k = 0; k < groups - 1; ++k){
                auto column = j + k;

                acc_width += widths[column];
                std::wcout << row[column] << std::wstring(widths[column] - row[column].length(), ' ');
            }

            //The last column of the group

            auto last_column = j + (groups - 1);
            auto width = widths[last_column];
            acc_width += width;

            //Pad with spaces to fit the header column width

            if(header_widths[j / groups] > acc_width){
                width += header_widths[j / groups] - acc_width;
            }

            //The very last column has no trailing space

            if(last_column == row.size() - 1){
                --width;
            }

            std::wcout << row[last_column] << std::wstring(width - row[last_column].length(), ' ');
        }

        std::wcout << format_code(0, 0, 7) << std::endl;
    }
}
