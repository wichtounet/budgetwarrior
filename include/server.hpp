//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <string>

#include "module_traits.hpp"

namespace budget {

struct server_module {
    void load();
    void handle(const std::vector<std::string>& args);
};

template<>
struct module_traits<server_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "server";
};

void set_server_running();
bool is_server_running();

} //end of namespace budget

#endif
