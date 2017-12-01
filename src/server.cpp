//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "server.hpp"
#include "writer.hpp"
#include "overview.hpp"
#include "expenses.hpp"
#include "accounts.hpp"

#include "httplib.h"

using namespace budget;

namespace {

static constexpr const char new_line = '\n';

std::string header(const std::string& title){
    std::stringstream stream;

    // The header

    stream << R"=====(
        <!doctype html>
        <html lang="en">
          <head>
            <meta charset="utf-8">
            <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
            <meta name="description" content="budgetwarrior">
            <meta name="author" content="Baptiste Wicht">
            <link href="https://getbootstrap.com/dist/css/bootstrap.min.css" rel="stylesheet">
            <style>
                body {
                  padding-top: 5rem;
                }

                .small-text {
                    font-size: 10pt;
                }

                .selector {
                    float: right;
                    font-size: xx-large;
                    pading-top: 5px;
                }
            </style>
    )=====";

    stream << "<title>budgetwarrior" << title << "</title>" << new_line;
    stream << "</head>" << new_line;
    stream << "<body>" << new_line;

    // The navigation

    stream << R"=====(
        <nav class="navbar navbar-expand-md navbar-dark bg-dark fixed-top">
          <a class="navbar-brand" href="#">budgetwarrior</a>
          <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarsExampleDefault" aria-controls="navbarsExampleDefault" aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
          </button>

          <div class="collapse navbar-collapse" id="navbarsExampleDefault">
            <ul class="navbar-nav mr-auto">
              <li class="nav-item active">
                <a class="nav-link" href="#">Index <span class="sr-only">(current)</span></a>
              </li>
              <li class="nav-item">
                <a class="nav-link" href="/overview/">Overview</a>
              </li>
              <li class="nav-item">
                <a class="nav-link" href="/portfolio/">Portfolio</a>
              </li>
            </ul>
          </div>
        </nav>
    )=====";

    // The main component

    stream << R"=====(<main role="main" class="container-fluid">)=====" << new_line;
    stream << "<div>" << new_line;

    return stream.str();
}

std::string footer(){
    std::stringstream stream;

    stream << R"=====(
        </div>
        </main>
        <script src="https://code.jquery.com/jquery-3.2.1.slim.min.js" integrity="sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN" crossorigin="anonymous"></script>
        <script>window.jQuery || document.write('<script src="https://getbootstrap.com/assets/js/vendor/jquery.min.js"><\/script>')</script>
        <script src="https://getbootstrap.com/assets/js/vendor/popper.min.js"></script>
        <script src="https://getbootstrap.com/dist/js/bootstrap.min.js"></script>
        </body>
        </html>
    )=====";

    return stream.str();
}

void index_page(const httplib::Request& req, httplib::Response& res){
    cpp_unused(req);

    std::stringstream content_stream;
    content_stream.imbue(std::locale("C"));

    content_stream << header("");

    // TODO

    content_stream << footer();

    res.set_content(content_stream.str(), "text/html");
}

void overview_page(const httplib::Request& req, httplib::Response& res){
    cpp_unused(req);

    std::stringstream content_stream;
    content_stream.imbue(std::locale("C"));

    content_stream << header("");

    budget::html_writer w(content_stream);

    if(req.matches.size() == 3){
        display_month_overview(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        display_month_overview(w);
    }

    content_stream << footer();

    res.set_content(content_stream.str(), "text/html");
}

} //end of anonymous namespace

void budget::server_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
}

void budget::server_module::handle(const std::vector<std::string>& args){
    cpp_unused(args);

    std::cout << "Starting the server" << std::endl;

    httplib::Server server;

    // Declare all the pages
    server.get("/", &index_page);

    server.get(R"(/overview/(\d+)/(\d+)/)", &overview_page);
    server.get("/overview/", &overview_page);

    server.set_error_handler([](const auto&, auto& res) {
        std::stringstream content_stream;
        content_stream.imbue(std::locale("C"));

        content_stream << header("");

        content_stream << "<p>Error Status: <span style='color:red;'>";
        content_stream << res.status;
        content_stream << "</span></p>";

        content_stream << footer();

        res.set_content(content_stream.str(), "text/html");
    });


    // Listen
    server.listen("localhost", 8080);
}
