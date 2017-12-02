//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"
#include "cpp_utils/string.hpp"

#include "writer.hpp"
#include "console.hpp"

budget::console_writer::console_writer(std::ostream& os) : os(os) {}

budget::writer& budget::console_writer::operator<<(const std::string& value){
    os << format(value);

    return *this;
}

budget::writer& budget::console_writer::operator<<(const double& value){
    os << value;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::money& m) {
    os << m;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::month& m) {
    os << m.as_short_string();

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::year& y) {
    os << y.value;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::end_of_line_t&) {
    os << std::endl;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::p_begin_t&) {
    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::p_end_t&) {
    os << std::endl;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::title_begin_t&) {
    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::title_end_t&) {
    os << std::endl << std::endl;

    return *this;
}

void budget::console_writer::display_table(std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents, size_t groups, std::vector<size_t> lines, size_t left){
    cpp_assert(groups > 0, "There must be at least 1 group");
    cpp_assert(contents.size() || columns.size(), "There must be at least some columns or contents");

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

    if (columns.empty()) {
        const size_t C = contents.front().size();
        for (size_t i = 0; i < C; ++i) {
            size_t width = 0;
            for (size_t j = i * groups; j < (i + 1) * groups; ++j) {
                width += widths[j];
            }

            header_widths.push_back(width + (i < C - 1 ? 1 : 0));
        }
    } else {
        for (size_t i = 0; i < columns.size(); ++i) {
            auto& column = columns[i];

            size_t width = 0;
            for (size_t j = i * groups; j < (i + 1) * groups; ++j) {
                width += widths[j];
            }

            width = std::max(width, rsize(column));
            header_widths.push_back(width + (i < columns.size() - 1 && rsize(column) >= width ? 1 : 0));

            //The last space is not underlined
            --width;

            os << format_code(4, 0, 7) << column << (width > rsize(column) ? std::string(width - rsize(column), ' ') : "") << format_code(0, 0, 7);

            //The very last column has no trailing space

            if (i < columns.size() - 1) {
                os << " ";
            }
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

    os << std::endl;
}
