//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"
#include "cpp_utils/string.hpp"

#include "writer.hpp"
#include "console.hpp"

namespace {

std::string success_to_string(unsigned long success) {
    std::stringstream ss;

    if (success < 25) {
        ss << "\033[0;31m";
    } else if (success < 75) {
        ss << "\033[0;33m";
    } else if (success < 100) {
        ss << "\033[0;32m";
    } else if (success >= 100) {
        ss << "\033[1;32m";
    }

    budget::print_minimum(ss, success, 5);
    ss << "%\033[0m  ";

    success     = std::min(success, 109UL);
    const size_t good = success == 0 ? 0 : (success / 10) + 1;

    for (size_t i = 0; i < good; ++i) {
        ss << "\033[1;42m   \033[0m";
    }

    for (size_t i = good; i < 11; ++i) {
        ss << "\033[1;41m   \033[0m";
    }

    return ss.str();
}

std::string format(std::string_view v) {
    if (v.starts_with("::red")) {
        auto value = v.substr(5);

        return "\033[0;31m" + std::string(value) + budget::format_reset();
    }
    if (v.starts_with("::green")) {
        auto value = v.substr(7);

        return "\033[0;32m" + std::string(value) + budget::format_reset();
    }

    if (v.starts_with("::blue")) {
        auto value = v.substr(6);

        return "\033[0;33m" + std::string(value) + budget::format_reset();
    }

    if (v.starts_with("::success")) {
        auto value = v.substr(9);
        auto success = budget::to_number<unsigned long>(value);
        return success_to_string(success);
    }

    return std::string(v);
}

std::string underline_format(std::string_view v) {
    if (v.starts_with("::red")) {
        auto value = v.substr(5);

        return "\033[4;31m" + std::string(value) + budget::format_reset();
    }

    if (v.starts_with("::green")) {
        auto value = v.substr(7);

        return "\033[4;32m" + std::string(value) + budget::format_reset();
    }

    if (v.starts_with("::blue")) {
        auto value = v.substr(6);

        return "\033[4;33m" + std::string(value) + budget::format_reset();
    }

    if (v.starts_with("::success")) {
        auto value = v.substr(9);
        auto success = budget::to_number<unsigned long>(value);
        return success_to_string(success);
    }

    return std::string(v);
}

} // end of anonymous namespace

budget::console_writer::console_writer(std::ostream& os)
        : os(os) {}

budget::writer& budget::console_writer::operator<<(std::string_view value) {
    os << ::format(value);

    return *this;
}

budget::writer& budget::console_writer::operator<<(double value) {
    os << value;
    return *this;
}
budget::writer& budget::console_writer::operator<<(size_t value) {
    os << value;
    return *this;
}
budget::writer& budget::console_writer::operator<<(long value) {
    os << value;
    return *this;
}
budget::writer& budget::console_writer::operator<<(int value) {
    os << value;
    return *this;
}
budget::writer& budget::console_writer::operator<<(unsigned value) {
    os << value;
    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::money& m) {
    os << m;

    return *this;
}

budget::writer& budget::console_writer::operator<<(const budget::day& d) {
    os << d.value;

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
    os << std::endl
       << std::endl;

    return *this;
}

void budget::console_writer::display_table(std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents, size_t groups, std::vector<size_t> lines, size_t left, [[maybe_unused]] size_t foot) {
    cpp_assert(groups > 0, "There must be at least 1 group");
    cpp_assert(!contents.empty() || !columns.empty(), "There must be at least some columns or contents");

    // Remove the Edit column, if necessary
    if (!columns.empty() && columns.back() == "Edit") {
        columns.pop_back();

        for (auto& row : contents) {
            row.pop_back();
        }
    }

    for (auto& row : contents) {
        for (auto& cell : row) {
            cpp::trim(cell);
        }
    }

    std::vector<size_t> widths;
    std::vector<size_t> header_widths;

    if (contents.empty()) {
        for (auto& column : columns) {
            widths.push_back(rsize(column));
        }
    } else {
        auto& first = contents[0];

        widths.assign(first.size(), 0);

        for (auto& row : contents) {
            for (size_t i = 0; i < row.size(); ++i) {
                widths[i] = std::max(widths[i], rsize(row[i]) + 1);
            }
        }
    }

    cpp_assert(columns.empty() || widths.size() == groups * columns.size(), "Widths incorrectly computed");

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

    for (size_t i = 0; i < contents.size(); ++i) {
        if (left) {
            os << std::string(left, ' ');
        }

        auto& row = contents[i];

        const bool underline = std::ranges::contains(lines, i);

        for (size_t j = 0; j < row.size(); j += groups) {
            size_t acc_width = 0;

            //First columns of the group
            for (size_t k = 0; k < groups - 1; ++k) {
                auto column = j + k;

                std::string const value = format(row[column]);

                acc_width += widths[column];

                if (underline) {
                    os << format_code(4, 0, 7);
                    os << underline_format(row[column]);
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
            auto width       = widths[last_column];
            acc_width += width;

            //Pad with spaces to fit the header column width

            if (header_widths[j / groups] > acc_width) {
                width += header_widths[j / groups] - acc_width;
            } else if (last_column == row.size() - 1) {
                --width;
            }

            auto value = format(row[last_column]);

            auto missing = width - rsize(row[last_column]);

            std::string fill_string;
            if (missing > 1) {
                fill_string = std::string(missing - 1, ' ');
            }

            if (underline) {
                os << format_code(4, 0, 7);
                os << underline_format(row[last_column]);
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

bool budget::console_writer::is_web() {
    return false;
}

void budget::console_writer::display_graph([[maybe_unused]] std::string_view                 title,
                                           [[maybe_unused]] std::vector<std::string>&        categories,
                                           [[maybe_unused]] std::vector<std::string>         series_names,
                                           [[maybe_unused]] std::vector<std::vector<float>>& series_values) {
    os << "unimplemented" << std::endl;
}
