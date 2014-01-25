//=======================================================================
// Copyright (c) 2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <sstream>

#include <boost/algorithm/string.hpp>

#include "console.hpp"
#include "assert.hpp"

std::string budget::format_code(int attr, int fg, int bg){
    std::stringstream stream;
    stream << "" << '\033' << "[" << attr << ";" << (fg + 30) << (bg + 40) << "m";
    return stream.str();
}

/**
 * Returns the real size of a string. By default, accented characteres are
 * represented by several chars and make the length of the string being bigger
 * than its displayable length. This functionr returns only a size of 1 for an
 * accented chars.
 * \param value The string we want the real length for.
 * \return The real length of the string.
 */
std::size_t rsize(const std::string& value){
    auto v = value;

    if(v.substr(0, 5) == "::red"){
        v = v.substr(5);
    } else if(v.substr(0, 7) == "::green"){
        v = v.substr(7);
    }

    static wchar_t buf[1025];

    return mbstowcs(buf, v.c_str(), 1024);
}

std::string budget::format(const std::string& v){
    if(v.substr(0, 5) == "::red"){
        auto value = v.substr(5);

        std::cout << "\033[0;31m";

        return value;
    } else if(v.substr(0, 7) == "::green"){
        auto value = v.substr(7);

        std::cout << "\033[0;32m";

        return value;
    }

    return v;
}

void budget::display_table(std::vector<std::string> columns, std::vector<std::vector<std::string>> contents, std::size_t groups){
    budget_assert(groups > 0, "There must be at least 1 group");

    for(auto& row : contents){
        for(auto& cell : row){
            boost::algorithm::trim(cell);
        }
    }

    std::vector<std::size_t> widths;
    std::vector<std::size_t> header_widths;

    if(!contents.size()){
        for(auto& column : columns){
            widths.push_back(rsize(column));
        }
    } else {
        auto& first = contents[0];

        widths.assign(first.size(), 0);

        for(auto& row : contents){
            for(std::size_t i = 0; i < row.size(); ++i){
                widths[i] = std::max(widths[i], rsize(row[i]) + 1);
            }
        }
    }

    budget_assert(widths.size() == groups * columns.size(), "Widths incorrectly computed");

    for(std::size_t i = 0; i < columns.size(); ++i){
        auto& column = columns[i];

        std::size_t width = 0;
        for(std::size_t j = i * groups; j < (i + 1) * groups; ++j){
            width += widths[j];
        }

        width = std::max(width, rsize(column));
        header_widths.push_back(width + (i < columns.size() - 1 && rsize(column) >= width ? 1 : 0));

        //The last space is not underlined
        --width;

        std::cout << format_code(4, 0, 7) << column << (width > rsize(column) ? std::string(width - rsize(column), ' ') : "") << format_code(0, 0, 7);

        //The very last column has no trailing space

        if(i < columns.size() - 1){
            std::cout << " ";
        }
    }

    std::cout << std::endl;

    for(std::size_t i = 0; i < contents.size(); ++i){
        auto& row = contents[i];

        std::cout << format_code(0, 0, 7);

        for(std::size_t j = 0; j < row.size(); j += groups){
            std::size_t acc_width = 0;

            //First columns of the group
            for(std::size_t k = 0; k < groups - 1; ++k){
                auto column = j + k;

                std::string value = format(row[column]);

                acc_width += widths[column];
                std::cout << value;

                //The content of the column can change the style, it is
                //important to reapply it

                std::cout << format_code(0, 0, 7);

                std::cout << std::string(widths[column] - rsize(value), ' ');
            }

            //The last column of the group

            auto last_column = j + (groups - 1);
            auto width = widths[last_column];
            acc_width += width;

            //Pad with spaces to fit the header column width

            if(header_widths[j / groups] > acc_width){
                width += header_widths[j / groups] - acc_width;
            } else if(last_column == row.size() - 1){
                --width;
            }

            auto value = format(row[last_column]);
            std::cout << value;

            //The content of the column can change the style, it is
            //important to reapply it

            std::cout << format_code(0, 0, 7);

            std::cout << std::string(width - rsize(row[last_column]), ' ');
        }

        std::cout << std::endl;
    }
}

void budget::edit_string(std::string& ref, const std::string& title){
    std::string answer;

    std::cout << title << " [" << ref << "]: ";
    std::getline(std::cin, answer);

    if(!answer.empty()){
        ref = answer;
    }
}

void budget::edit_money(budget::money& ref, const std::string& title){
    std::string answer;

    std::cout << title << " [" << ref << "]: ";
    std::getline(std::cin, answer);

    if(!answer.empty()){
        ref = parse_money(answer);
    }
}

void budget::edit_date(boost::gregorian::date& ref, const std::string& title){
    std::string answer;

    std::cout << title << " [" << ref << "]: ";
    std::getline(std::cin, answer);

    if(!answer.empty()){
        ref = boost::gregorian::from_string(answer);
    }
}
