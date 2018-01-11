//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef SERVER_PAGES_H
#define SERVER_PAGES_H

namespace httplib {
struct Server;
};

namespace budget {

void load_pages(httplib::Server& server);

} //end of namespace budget

#endif
