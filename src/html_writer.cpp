//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"
#include "cpp_utils/string.hpp"

#include "writer.hpp"
#include "console.hpp"
#include "expenses.hpp"
#include "earnings.hpp"

namespace {

std::string success_to_string(int success) {
    success = std::min(success, 100);
    success = std::max(success, 0);

    std::stringstream ss;

    ss << R"=====(<div class="progress">)=====";
    ss << R"=====(<div class="progress-bar" role="progressbar" style="width:)=====" << success << R"=====(%;" aria-valuenow=")=====" << success << R"=====(" aria-valuemin="0" aria-valuemax="100">)=====" << success << R"=====(%</div>)=====";
    ss << R"=====(</div>)=====";

    return ss.str();
}

std::string edit_to_string(const std::string& module, const std::string& id){
    std::stringstream ss;

    std::string delete_url = "/api/" + module + "/delete/?server=yes&back_page=__budget_this_page__&input_id=" + id;
    std::string edit_url = "/api/" + module + "/edit/?server=yes&back_page=__budget_this_page__&input_id=" + id;

    // Add the delete button
    ss << R"=====(<a href=")=====";
    ss << delete_url;
    ss << R"=====(">)=====";
    ss << R"=====(<button type="button" aria-label="Delete" class="btn btn-sm btn-danger oi oi-circle-x"></button>)=====";
    ss << R"=====(</a>)=====";

    // Add the edit button
    ss << R"=====(<a href=")=====";
    ss << edit_url;
    ss << R"=====(">)=====";
    ss << R"=====(<button type="submit" aria-label="Edit" class="btn btn-sm btn-warning oi oi-pencil"></button>)=====";
    ss << R"=====(</a>)=====";

    return ss.str();
}

std::string html_format(budget::html_writer& w, const std::string& v){
    if(v.substr(0, 5) == "::red"){
        auto value = v.substr(5);

        return "<span style=\"color:red;\">" + value + "</span>";
    } else if(v.substr(0, 6) == "::blue"){
        auto value = v.substr(6);

        return "<span style=\"color:blue;\">" + value + "</span>";
    } else if(v.substr(0, 7) == "::green"){
        auto value = v.substr(7);

        return "<span style=\"color:green;\">" + value + "</span>";
    } else if(v.substr(0, 9) == "::success"){
        auto value = v.substr(9);
        auto success = budget::to_number<unsigned long>(value);
        return success_to_string(success);
    } else if(v.substr(0, 8) == "::edit::"){
        auto value = v.substr(8);

        if(value.find("::") == std::string::npos){
            return v;
        }

        auto module = value.substr(0, value.find("::"));
        auto id     = value.substr(value.find("::") + 2, value.size());

        w.use_module("open-iconic");

        return edit_to_string(module, id);
    }

    return v;
}

} // end of anonymous namespace

budget::html_writer::html_writer(std::stringstream& os) : os(os) {}

budget::writer& budget::html_writer::operator<<(const std::string& value){
    os << html_format(*this, value);

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
    title_started = true;

    os << R"======(<div class="row">)======";
    os << R"======(<div class="col-auto">)======";
    os << R"======(<h2>)======";

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::title_end_t&) {
    if (title_started) {
        os << "</h2>";  // end of the title
        os << "</div>"; // end of the col
    }

    title_started = false;

    os << "</div>"; //end of the row

    return *this;
}

std::vector<budget::year> active_years(){
    std::vector<budget::year> years;

    for (auto& expense : budget::all_expenses()) {
        if (expense.date != budget::TEMPLATE_DATE) {
            auto y = expense.date.year();

            if (std::find(years.begin(), years.end(), y) == years.end()) {
                years.push_back(y);
            }
        }
    }

    for (auto& earning : budget::all_earnings()) {
        if (earning.date != budget::TEMPLATE_DATE) {
            auto y = earning.date.year();

            if (std::find(years.begin(), years.end(), y) == years.end()) {
                years.push_back(y);
            }
        }
    }

    return years;
}

budget::writer& budget::html_writer::operator<<(const budget::year_month_selector& m) {
    if (title_started) {
        os << "</h2>";  // end of the title
        os << "</div>"; // end of the col
    }

    title_started = false;

    os << R"======(<div class="col selector text-right">)======";

    auto previous_month = m.current_month;
    auto previous_year = m.current_year;
    auto next_month = m.current_month;
    auto next_year = m.current_year;

    if (m.current_month == 1) {
        previous_month = 12;
        previous_year  = m.current_year - 1;
    } else {
        previous_month = m.current_month - 1;
        previous_year  = m.current_year;
    }

    if (m.current_month == 12) {
        next_month = 1;
        next_year  = m.current_year + 1;
    } else {
        next_month = m.current_month + 1;
        next_year  = m.current_year;
    }

    os << "<a aria-label=\"Previous\" href=\"/" << m.page << "/" << previous_year << "/" << previous_month.value << "/\"><span class=\"oi oi-arrow-thick-left\"></span></a>";

    os << "<select aria-label=\"Month\" id=\"month_selector\">";
    for (size_t i = 1; i < 13; ++i) {
        if (i == m.current_month) {
            os << "<option selected>" << i << "</option>";
        } else {
            os << "<option>" << i << "</option>";
        }
    }
    os << "</select>";

    os << "<select aria-label=\"Year\" id=\"year_selector\">";

    auto years = active_years();

    if(std::find(years.begin(), years.end(), m.current_year) == years.end()){
        years.push_back(m.current_year);
    }

    std::sort(years.begin(), years.end());

    for(auto year : years){
        if(year == m.current_year){
            os << "<option selected>" << year << "</option>";
        } else {
            os << "<option>" << year << "</option>";
        }
    }

    os << "</select>";

    os << "<a aria-label=\"Next\" href=\"/" << m.page << "/" << next_year << "/" << next_month.value << "/\"><span class=\"oi oi-arrow-thick-right\"></span></a>";

    os << "</div>";

    std::stringstream ss;

    ss << "var update_page = function(){";
    ss << "var selected_year = $(\"#year_selector\").find(':selected');";
    ss << "var selected_month = $(\"#month_selector\").find(':selected');";
    ss << "window.location = \"/" << m.page << "/" << "\" + selected_year.val() + \"/\" + selected_month.val() + \"/\";";
    ss << "};";
    ss << "$('#year_selector').change(update_page);";
    ss << "$('#month_selector').change(update_page);";

    defer_script(ss.str());

    use_module("open-iconic");

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::year_selector& m) {
    if (title_started) {
        os << "</h2>";  // end of the title
        os << "</div>"; // end of the col
    }

    title_started = false;

    os << R"======(<div class="col selector text-right">)======";

    auto previous_year = m.current_year - 1;
    auto next_year     = m.current_year + 1;

    os << "<a aria-label=\"Previous\" href=\"/" << m.page << "/" << previous_year << "/\"><span class=\"oi oi-arrow-thick-left\"></span></a>";
    os << "<select aria-label=\"Year\" id=\"year_selector\">";

    auto years = active_years();

    if(std::find(years.begin(), years.end(), m.current_year) == years.end()){
        years.push_back(m.current_year);
    }

    std::sort(years.begin(), years.end());

    for(auto year : years){
        if(year == m.current_year){
            os << "<option selected>" << year << "</option>";
        } else {
            os << "<option>" << year << "</option>";
        }
    }

    os << "</select>";
    os << "<a aria-label=\"Next\" href=\"/" << m.page << "/" << next_year << "/\"><span class=\"oi oi-arrow-thick-right\"></span></a>";

    os << "</div>";

    std::stringstream ss;

    ss << "$('#year_selector').change(function(){";
    ss << "var selected = $(this).find(':selected');";
    ss << "window.location = \"/" << m.page << "/" << "\" + selected.val() + \"/\";";
    ss << "})";

    defer_script(ss.str());

    use_module("open-iconic");

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::asset_selector& m) {
    if (title_started) {
        os << "</h2>";  // end of the title
        os << "</div>"; // end of the col
    }

    title_started = false;

    os << R"======(<div class="col selector text-right">)======";

    auto assets = all_user_assets().to_vector();

    size_t previous_asset = 0;
    size_t next_asset = 0;

    for (size_t i = 0; i < assets.size(); ++i) {
        auto & asset = assets[i];

        if (asset.id == m.current_asset) {
            next_asset = assets[(i + 1) % assets.size()].id;
            previous_asset = assets[i > 0 ? i - 1 : assets.size() - 1].id;
            break;
        }
    }

    os << "<a aria-label=\"Previous\" href=\"/" << m.page << "/" << previous_asset << "/\"><span class=\"oi oi-arrow-thick-left\"></span></a>";
    os << "<select aria-label=\"Year\" id=\"asset_selector\">";

    for(auto asset : assets){
        if(asset.id == m.current_asset){
            os << "<option value=" << asset.id << " selected>" << asset.name << "</option>";
        } else {
            os << "<option value=" << asset.id << ">" << asset.name << "</option>";
        }
    }

    os << "</select>";
    os << "<a aria-label=\"Next\" href=\"/" << m.page << "/" << next_asset << "/\"><span class=\"oi oi-arrow-thick-right\"></span></a>";

    os << "</div>";

    std::stringstream ss;

    ss << "$('#asset_selector').change(function(){";
    ss << "var selected = $(this).find(':selected');";
    ss << "window.location = \"/" << m.page << "/" << "\" + selected.val() + \"/\";";
    ss << "})";

    defer_script(ss.str());

    use_module("open-iconic");

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::add_button& b) {
    if (title_started) {
        os << "</h2>";  // end of the title
        os << "</div>"; // end of the col
    }

    title_started = false;

    os << R"======(<div class="col-auto">)======";
    os << "<a href=\"/" << b.module << "/add/\" class=\"btn btn-info\" role=\"button\">New</a>\n";
    os << R"======(</div>)======";

    return *this;
}

budget::writer& budget::html_writer::operator<<(const budget::set_button& b) {
    if (title_started) {
        os << "</h2>";  // end of the title
        os << "</div>"; // end of the col
    }

    title_started = false;

    os << R"======(<div class="col-auto">)======";
    os << "<a href=\"/" << b.module << "/set/\" class=\"btn btn-info\" role=\"button\">Set</a>\n";
    os << R"======(</div>)======";

    return *this;
}

void budget::html_writer::display_table(std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents, size_t groups, std::vector<size_t> lines, size_t left, size_t foot){
    cpp_assert(groups > 0, "There must be at least 1 group");
    cpp_unused(left);
    cpp_unused(lines);

    for (auto& row : contents) {
        if (row.size() < columns.size()) {
            std::cerr << "Invalid number of columns in row" << std::endl;
            return;
        }

        for (auto& cell : row) {
            cpp::trim(cell);
        }
    }

    size_t extend = columns.size();
    size_t edit = columns.size();

    for (size_t i = 0; i < columns.size(); ++i) {
        for (auto& row : contents) {
            if (row[i].size() >= 9 && row[i].substr(0, 9) == "::success") {
                extend = i;
                break;
            }

            if (row[i].size() >= 6 && row[i].substr(0, 6) == "::edit") {
                edit = i;
                break;
            }
        }

        if (extend == i || edit == i) {
            break;
        }
    }

    bool small = columns.empty(); // TODO Improve this heuristic!

    if(small){
        os << "<div class=\"row\">";
        os << "<div class=\"col-md-4\">&nbsp;</div>";
        os << "<div class=\"col-md-4\">";
    } else {
        os << "<div class=\"table-responsive\">";
    }

    os << "<table class=\"table table-sm small-text\">";

    // Display the header

    if (columns.size()) {
        os << "<thead>";
        os << "<tr>";

        for (size_t i = 0; i < columns.size(); ++i) {
            auto& column = columns[i];

            if(column == "ID"){
                continue;
            }

            std::string style;

            // TODO: This is only a bad hack, at best
            if(i == extend){
                style = " class=\"extend-only\"";
            }

            if(i == edit){
                style = " class=\"not-sortable\"";
            }

            if (groups > 1) {
                os << "<th colspan=\"" << groups << "\"" << style << ">" << column << "</th>";
            } else {
                os << "<th" << style << ">" << column << "</th>";
            }
        }

        os << "</tr>";
        os << "</thead>";
    }

    // Display the contents

    os << "<tbody>";

    for(size_t i = 0; i < contents.size() - foot; ++i){
        auto& row = contents[i];

        os << "<tr>";

        for(size_t j = 0; j < row.size(); ++j){
            if (columns.size() && groups == 1 && columns[j] == "ID") {
                continue;
            }

            std::string value = html_format(*this, row[j]);

            if(value.empty()){
                os << "<td>&nbsp;</td>";
            } else {
                if(columns.empty() && j == 0){
                    os << "<th scope=\"row\">" << value << "</th>";
                } else {
                    os << "<td>" << value << "</td>";
                }
            }
        }

        os << "</tr>";
    }

    os << "</tbody>";

    if (foot) {
        os << "<tfoot>";

        for (size_t i = contents.size() - foot; i < contents.size(); ++i) {
            auto& row = contents[i];

            os << "<tr>";

            for (size_t j = 0; j < row.size(); ++j) {
                if (columns.size() && groups == 1 && columns[j] == "ID") {
                    continue;
                }

                std::string value = html_format(*this, row[j]);

                if (value.empty()) {
                    os << "<td>&nbsp;</td>";
                } else {
                    os << "<td>" << value << "</td>";
                }
            }

            os << "</tr>";
        }

        os << "</tfoot>";
    }

    os << "</table>";

    if (small) {
        os << "</div>"; // middle column
        os << "<div class=\"col-md-4\">&nbsp;</div>";
        os << "</div>"; // row
    } else {
        os << "</div>"; // table-responsive
    }
}

bool budget::html_writer::is_web() {
    return true;
}

void budget::html_writer::display_graph(const std::string& title, std::vector<std::string>& categories, std::vector<std::string> series_names, std::vector<std::vector<float>>& series_values){
    use_module("highcharts");

    os << R"=====(<div id="container" style="min-width: 310px; height: 400px; margin: 0 auto"></div>)=====";

    std::stringstream ss;
    ss.imbue(std::locale("C"));

    ss << R"=====(Highcharts.chart('container', {)=====";
    ss << R"=====(chart: {type: 'column'},)=====";
    ss << R"=====(credits: {enabled: true},)=====";

    ss << "title: { text: '" << title << "'},";
    ss << "xAxis: { categories: [";

    for (auto& category : categories) {
        ss << "'" << category << "',";
    }

    ss << "]},";

    ss << "series: [";

    for(size_t i = 0; i < series_names.size(); ++i){
        ss << "{ name: '" << series_names[i] << "',";
        ss << "data: [";

        for(auto& value : series_values[i]){
            ss << value << ",";
        }

        ss << "]},";
    }

    ss << "]";

    ss << R"=====(});)=====";

    defer_script(ss.str());
}

void budget::html_writer::defer_script(const std::string& script){
    std::stringstream ss;

    ss << R"=====(<script>)=====" << '\n';
    ss << R"=====($(function(){)=====" << '\n';
    ss << script;
    ss << R"=====(});)=====";
    ss << R"=====(</script>)=====";

    scripts.emplace_back(ss.str());
}

void budget::html_writer::load_deferred_scripts(){
    // The javascript for Boostrap and JQuery
    os << R"=====(
            <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery.slim.min.js" integrity="sha256-3edrmyuQ0w65f8gfBsqowzjJe2iM6n0nKciPUp8y+7E=" crossorigin="anonymous"></script>
            <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.13.0/umd/popper.min.js" integrity="sha256-pS96pU17yq+gVu4KBQJi38VpSuKN7otMrDQprzf/DWY=" crossorigin="anonymous"></script>
            <script src="https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/4.0.0-beta.3/js/bootstrap.min.js" integrity="sha256-JNyuT3QsYBdyeKxKBwnGJAJiACWcow2TjhNruIFFPMQ=" crossorigin="anonymous"></script>
    )=====";

    // Open-Iconic
    if (need_module("open-iconic")) {
        os << R"=====(<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/open-iconic/1.1.1/font/css/open-iconic-bootstrap.min.css" integrity="sha256-BJ/G+e+y7bQdrYkS2RBTyNfBHpA9IuGaPmf9htub5MQ=" crossorigin="anonymous" />)=====";
    }

    // DataTables
    if (need_module("datatables")) {
        os << R"=====(
            <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/datatables/1.10.16/css/dataTables.bootstrap4.min.css" integrity="sha256-LpykTdjMm+jVLpDWiYOkH8bYiithb4gajMYnIngj128=" crossorigin="anonymous" />
            <script src="https://cdnjs.cloudflare.com/ajax/libs/datatables/1.10.16/js/jquery.dataTables.min.js" integrity="sha256-qcV1wr+bn4NoBtxYqghmy1WIBvxeoe8vQlCowLG+cng=" crossorigin="anonymous"></script>
            <script src="https://cdnjs.cloudflare.com/ajax/libs/datatables/1.10.16/js/dataTables.bootstrap4.min.js" integrity="sha256-PahDJkda1lmviWgqffy4CcrECIFPJCWoa9EAqVx7Tf8=" crossorigin="anonymous"></script>
        )=====";
    }

    // Highcharts
    if (need_module("highcharts")) {
        os << R"=====(
            <script src="https://cdnjs.cloudflare.com/ajax/libs/highcharts/6.0.4/highstock.js" integrity="sha256-ZdoT00QjMb+DdcNmKfZJYcZY6/H83RYoLS4sZRL43T8=" crossorigin="anonymous"></script>
            <script src="https://cdnjs.cloudflare.com/ajax/libs/highcharts/6.0.4/highcharts-more.js" integrity="sha256-QnoLQZe7BYRVTl3AY8Lsw6mn60HfHZNpcZBEndybfBk=" crossorigin="anonymous"></script>
            <script src="https://cdnjs.cloudflare.com/ajax/libs/highcharts/6.0.4/js/modules/solid-gauge.js" integrity="sha256-AIfWX+axQ036B1bKbqeWxklZ4BILxbfcNKDh+sqFS+g=" crossorigin="anonymous"></script>
            <script src="https://cdnjs.cloudflare.com/ajax/libs/highcharts/6.0.4/js/modules/series-label.js" integrity="sha256-58Ca6fVLKQfXdNwnmkcPq09InNJa/Io8EJPJtKXT70g=" crossorigin="anonymous"></script>
        )=====";
    }

    // Add the custom scripts
    for (auto& script : scripts) {
        os << script << '\n';
    }
}

void budget::html_writer::use_module(const std::string& module){
    if (std::find(modules.begin(), modules.end(), module) == modules.end()) {
        modules.push_back(module);
    }
}

bool budget::html_writer::need_module(const std::string& module){
    return std::find(modules.begin(), modules.end(), module) != modules.end();
}
