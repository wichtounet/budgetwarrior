//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <sstream>

#include "cpp_utils/assert.hpp"
#include "cpp_utils/string.hpp"

#include "console.hpp"

std::string budget::format_code(int attr, int fg, int bg){
    std::stringstream stream;
    stream << "" << '\033' << "[" << attr << ";" << (fg + 30) << (bg + 40) << "m";
    return stream.str();
}

std::string budget::format_reset(){
    return format_code(0, 0, 7);
}

std::string budget::format_money(const budget::money& m){
    if(m.positive()){
        return "::green" + budget::to_string(m);
    } else if(m.negative()){
        return "::red" + budget::to_string(m);
    } else {
        return budget::to_string(m);
    }
}

std::string budget::format_money_no_color(const budget::money& m){
    return budget::to_string(m);
}

std::string budget::format_money_reverse(const budget::money& m){
    if(m.positive()){
        return "::red" + budget::to_string(m);
    } else if(m.negative()){
        return "::green" + budget::to_string(m);
    } else {
        return budget::to_string(m);
    }
}

std::size_t budget::rsize(const std::string& value){
    auto v = value;

    if(v.substr(0, 5) == "::red"){
        v = v.substr(5);
    } else if(v.substr(0, 7) == "::green"){
        v = v.substr(7);
    }

    static wchar_t buf[1025];
    return mbstowcs(buf, v.c_str(), 1024);
}

std::size_t budget::rsize_after(const std::string& value){
    auto v = value;

    auto index = v.find('\033');

    while(index != std::string::npos){
        if (v[index + 3] == 'm') {
            v.erase(index, 4);
        } else if (v[index + 6] == 'm') {
            v.erase(index, 7);
        } else {
            v.erase(index, 9);
        }

        index = v.find('\033');
    }

    static wchar_t buf[1025];
    return mbstowcs(buf, v.c_str(), 1024);
}

bool budget::option(const std::string& option, std::vector<std::string>& args){
    auto before = args.size();
    args.erase(std::remove(args.begin(), args.end(), option), args.end());
    return before != args.size();
}

std::string budget::option_value(const std::string& option, std::vector<std::string>& args, const std::string& default_value){
    auto it = args.begin();
    auto end = args.end();

    auto value = default_value;

    while(it != end){
        if (it->find(option + "=") == 0){
            value = std::string(it->begin() + option.size() + 1, it->end());

            it = args.erase(it);
            end = args.end();
        } else {
            ++it;
        }
    }

    return value;
}

std::string budget::format(const std::string& v){
    if(v.substr(0, 5) == "::red"){
        auto value = v.substr(5);

        return "\033[0;31m" + value + format_reset();
    } else if(v.substr(0, 7) == "::green"){
        auto value = v.substr(7);

        return "\033[0;32m" + value + format_reset();
    }

    return v;
}

void budget::display_table(std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents, std::size_t groups){
    display_table(std::cout, columns, contents, groups);
}

void budget::display_table(std::ostream& os, std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents, std::size_t groups){
    cpp_assert(groups > 0, "There must be at least 1 group");

    for(auto& row : contents){
        for(auto& cell : row){
            cpp::trim(cell);
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

    cpp_assert(widths.size() == groups * columns.size(), "Widths incorrectly computed");

    // Display the header

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

        os << format_code(4, 0, 7) << column << (width > rsize(column) ? std::string(width - rsize(column), ' ') : "") << format_code(0, 0, 7);

        //The very last column has no trailing space

        if(i < columns.size() - 1){
            os << " ";
        }
    }

    os << std::endl;

    // Display the contents

    for(std::size_t i = 0; i < contents.size(); ++i){
        auto& row = contents[i];

        os << format_code(0, 0, 7);

        for(std::size_t j = 0; j < row.size(); j += groups){
            std::size_t acc_width = 0;

            //First columns of the group
            for(std::size_t k = 0; k < groups - 1; ++k){
                auto column = j + k;

                std::string value = format(row[column]);

                acc_width += widths[column];
                os << value;

                os << std::string(widths[column] - rsize(value), ' ');
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
            os << value;

            os << std::string(width - rsize(row[last_column]), ' ');
        }

        os << std::endl;
    }
}
