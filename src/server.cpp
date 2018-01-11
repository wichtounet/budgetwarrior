//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "cpp_utils/assert.hpp"

#include "server.hpp"
#include "writer.hpp"
#include "overview.hpp"
#include "expenses.hpp"
#include "accounts.hpp"
#include "assets.hpp"
#include "config.hpp"
#include "objectives.hpp"
#include "wishes.hpp"
#include "version.hpp"
#include "summary.hpp"
#include "fortune.hpp"
#include "recurring.hpp"
#include "guid.hpp"
#include "report.hpp"
#include "debts.hpp"

#include "server_api.hpp"
#include "server_pages.hpp"

#include "httplib.h"

using namespace budget;

namespace {

bool server_running = false;

} //end of anonymous namespace

void budget::set_server_running(){
    // Indicates to the system that it's running in server mode
    server_running = true;
}

void budget::server_module::load(){
    load_accounts();
    load_expenses();
    load_earnings();
    load_assets();
    load_objectives();
    load_wishes();
    load_fortunes();
    load_recurrings();
    load_debts();
    load_wishes();
}

void budget::server_module::handle(const std::vector<std::string>& args){
    cpp_unused(args);

    std::cout << "Starting the server" << std::endl;

    httplib::Server server;

    load_pages(server);
    load_api(server);

    std::string listen = "localhost";
    size_t port = 8080;

    if(config_contains("server_port")){
        port = to_number<size_t>(config_value("server_port"));
    }

    if(config_contains("server_listen")){
        listen = config_value("server_listen");
    }

    // Listen
    server.listen(listen.c_str(), port);
}

bool budget::is_server_running(){
    return server_running;
}
