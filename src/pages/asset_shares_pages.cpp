//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "assets.hpp"

#include "writer.hpp"
#include "pages/asset_shares_pages.hpp"
#include "http.hpp"

using namespace budget;

void budget::list_asset_shares_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "List asset shares")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::list_asset_shares(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}
