//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "pages/server_pages.hpp"
#include "writer.hpp"
#include "http.hpp"
#include "report.hpp"

using namespace budget;

void budget::report_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Report")) {
        return;
    }

    budget::html_writer w(content_stream);

    auto today = budget::local_day();
    report(w, today.year(), false, "");

    page_end(w, req, res);
}
