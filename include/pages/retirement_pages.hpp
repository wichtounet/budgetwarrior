//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

void retirement_status_page(const httplib::Request& req, httplib::Response& res);
void retirement_configure_page(const httplib::Request& req, httplib::Response& res);
void retirement_fi_ratio_over_time(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
