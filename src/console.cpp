//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <sstream>

#include "cpp_utils/assert.hpp"
#include "cpp_utils/string.hpp"

#include "console.hpp"

// For getch
#include <termios.h>
#include <unistd.h>

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

size_t budget::rsize(const std::string& value){
    auto v = value;

    if(v.substr(0, 5) == "::red"){
        v = v.substr(5);
    } else if(v.substr(0, 7) == "::green"){
        v = v.substr(7);
    }

    static wchar_t buf[1025];
    return mbstowcs(buf, v.c_str(), 1024);
}

size_t budget::rsize_after(const std::string& value){
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

void budget::display_table(std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents, size_t groups, std::vector<size_t> lines, size_t left){
    display_table(std::cout, columns, contents, groups, lines, left);
}

void budget::display_table(std::ostream& os, std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents, size_t groups, std::vector<size_t> lines, size_t left){
    cpp_assert(groups > 0, "There must be at least 1 group");

    for(auto& row : contents){
        for(auto& cell : row){
            cpp::trim(cell);
        }
    }

    std::vector<size_t> widths;
    std::vector<size_t> header_widths;

    if(!contents.size()){
        for(auto& column : columns){
            widths.push_back(rsize(column));
        }
    } else {
        auto& first = contents[0];

        widths.assign(first.size(), 0);

        for(auto& row : contents){
            for(size_t i = 0; i < row.size(); ++i){
                widths[i] = std::max(widths[i], rsize(row[i]) + 1);
            }
        }
    }

    cpp_assert(widths.size() == groups * columns.size(), "Widths incorrectly computed");

    // Display the header

    if (left) {
        os << std::string(left, ' ');
    }

    for(size_t i = 0; i < columns.size(); ++i){
        auto& column = columns[i];

        size_t width = 0;
        for(size_t j = i * groups; j < (i + 1) * groups; ++j){
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

    for(size_t i = 0; i < contents.size(); ++i){
        if(left){
            os << std::string(left, ' ');
        }

        auto& row = contents[i];

        bool underline = std::find(lines.begin(), lines.end(), i) != lines.end();

        for(size_t j = 0; j < row.size(); j += groups){
            size_t acc_width = 0;

            //First columns of the group
            for(size_t k = 0; k < groups - 1; ++k){
                auto column = j + k;

                std::string value = format(row[column]);

                acc_width += widths[column];

                if (underline) {
                    os << format_code(4, 0, 7);
                    os << value;
                    os << std::string(widths[column] - rsize(value) - 1, ' ');
                    os << format_code(0, 0, 7);
                } else {
                    os << value;
                    os << std::string(widths[column] - rsize(value) - 1, ' ');
                }

                os << ' ';
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

            auto missing = width - rsize(row[last_column]);

            std::string fill_string;
            if (missing > 1){
                fill_string = std::string(missing - 1, ' ');
            }

            if (underline) {
                os << format_code(4, 0, 7);
                os << value;
                os << fill_string;
                os << format_code(0, 0, 7);
            } else {
                os << value;
                os << fill_string;
            }

            if (missing > 0) {
                if (j == row.size() - 1 && underline) {
                    os << format_code(4, 0, 7);
                    os << ' ';
                    os << format_code(0, 0, 7);
                } else {
                    os << ' ';
                }
            }
        }

        os << format_code(0, 0, 7) << std::endl;
    }
}

namespace {

char getch() {
    char buf = 0;
    struct termios old;
    fflush(stdout);

    if (tcgetattr(0, &old) < 0) {
        perror("tcsetattr()");
    }

    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN]  = 1;
    old.c_cc[VTIME] = 0;

    if (tcsetattr(0, TCSANOW, &old) < 0) {
        perror("tcsetattr ICANON");
    }

    if (read(0, &buf, 1) < 0) {
        perror("read()");
    }

    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;

    if (tcsetattr(0, TCSADRAIN, &old) < 0) {
        perror("tcsetattr ~ICANON");
    }

    return buf;
}

} // end of anonymous namespace

std::string budget::get_string_complete(const std::vector<std::string>& choices){
    std::string answer;

    if(choices.empty()){
        std::getline(std::cin, answer);
        return answer;
    }

    size_t index = 0;

    while(true){
        char c = getch();

        if (+c == 127){
            if(!answer.empty()){
                std::cout << "\b \b";
                answer.pop_back();
            }
        } else if (c == '\r'){
            std::cout << std::endl;
            return answer;
        } else if (c == '\n'){
            std::cout << std::endl;
            return answer;
        } else if(c == '\033'){
            getch();

            char cc = getch();

            if(cc == 'A'){
                for(size_t i = 0; i < answer.size(); ++i){
                    std::cout << "\b \b";
                }

                index = (index + 1) % (choices.size() + 1);

                if(index > 0){
                    answer = choices[index - 1];
                } else if(index == 0){
                    answer = "";
                }

                std::cout << answer;
            } else if(cc == 'B'){
                for(size_t i = 0; i < answer.size(); ++i){
                    std::cout << "\b \b";
                }

                if(index == 1){
                    index = 0;
                    answer = "";
                } else if(index == 0){
                    index = choices.size();
                    answer = choices[index - 1];
                } else {
                    --index;
                    answer = choices[index - 1];
                }

                std::cout << answer;
            }
        } else {
            std::cout << c;
            answer += c;
        }
    }

    return answer;
}
