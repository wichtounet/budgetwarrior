//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "server.hpp"

#include "httplib.h"

using namespace budget;

namespace {

static constexpr const char new_line = '\n';

std::string header(const std::string& title){
    std::stringstream stream;

    stream << "<html>" << new_line;
    stream << "<head>" << new_line;
    stream << "<title>budgetwarrior" << title << "</title>" << new_line;
    stream << "</head>" << new_line;
    stream << "<body>" << new_line;
    stream << "</body>" << new_line;
    stream << "</html>" << new_line;

    return stream.str();
}

std::string footer(){
    std::stringstream stream;

    stream << "</body>" << new_line;
    stream << "</html>" << new_line;

    return stream.str();
}

void index_page(const httplib::Request& req, httplib::Response& res){
    cpp_unused(req);

    std::stringstream content_stream;

    content_stream << header("");



    content_stream << footer();

    res.set_content(content_stream.str(), "text/html");
}

} //end of anonymous namespace

void budget::server_module::handle(const std::vector<std::string>& args){
    cpp_unused(args);

    std::cout << "Starting the server" << std::endl;

    httplib::Server svr;

    // Declare all the pages
    svr.get("/", &index_page);

    // Listen
    svr.listen("localhost", 8080);
}
