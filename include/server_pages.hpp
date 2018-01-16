//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

namespace httplib {
struct Server;
};

namespace budget {

void load_pages(httplib::Server& server);

} //end of namespace budget
