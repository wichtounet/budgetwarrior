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

namespace {

std::string html_format(const std::string& v){
    if(v.substr(0, 5) == "::red"){
        auto value = v.substr(5);

        return "<span style=\"color:red;\">" + value + "</span>";
    } else if(v.substr(0, 7) == "::green"){
        auto value = v.substr(7);

        return "<span style=\"color:green;\">" + value + "</span>";
    }

    return v;
}

} // end of anonymous namespace

budget::html_writer::html_writer(std::ostream& os) : os(os) {}

budget::writer& budget::html_writer::operator<<(const std::string& value){
    os << html_format(value);

    return *this;
}

budget::writer& budget::html_writer::operator<<(const double& value){
    os << value;

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::money& m) {
    os << m;

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::month& m) {
    os << m.as_short_string();

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::year& y) {
    os << y.value;

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::end_of_line_t&) {
    os << "\n";
    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::p_begin_t&) {
    os << "<p>";

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::p_end_t&) {
    os << "</p>";

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::title_begin_t&) {
    os << "<h1>";

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::title_end_t&) {
    os << "</h2>";

    return *this;
}

void budget::html_writer::display_table(std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents, size_t groups, std::vector<size_t> lines, size_t left){
    cpp_assert(groups > 0, "There must be at least 1 group");
    cpp_unused(left);
    cpp_unused(lines);

    for(auto& row : contents){
        for(auto& cell : row){
            cpp::trim(cell);
        }
    }

    os << "<table class=\"table table-sm small-text\">";

    cpp_assert(widths.size() == groups * columns.size(), "Widths incorrectly computed");

    // Display the header

    os << "<thead>";
    os << "<tr>";

    for(size_t i = 0; i < columns.size(); ++i){
        auto& column = columns[i];

        if (groups > 1) {
            os << "<th colspan=\"" << groups << "\">" << column << "</th>";
        } else {
            os << "<th>" << column << "</th>";
        }
    }

    os << "</tr>";
    os << "</thead>";

    // Display the contents

    os << "<tbody>";

    for(size_t i = 0; i < contents.size(); ++i){
        auto& row = contents[i];

        os << "<tr>";

        for(size_t j = 0; j < row.size(); ++j){
            std::string value = html_format(row[j]);

            if(value.empty()){
                os << "<td>&nbsp;</td>";
            } else {
                os << "<td>" << value << "</td>";
            }
        }

        os << "</tr>";
    }

    os << "</tbody>";

    os << "</table>";
}
