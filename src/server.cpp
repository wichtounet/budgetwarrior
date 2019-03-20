//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>
#include <thread>

#include "cpp_utils/assert.hpp"

#include "server.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "accounts.hpp"
#include "assets.hpp"
#include "config.hpp"
#include "objectives.hpp"
#include "wishes.hpp"
#include "fortune.hpp"
#include "recurring.hpp"
#include "debts.hpp"
#include "currency.hpp"
#include "share.hpp"
#include "http.hpp"

#include "api/server_api.hpp"
#include "pages/server_pages.hpp"

using namespace budget;

namespace {

bool server_running = false;

void start_server(){
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

void start_cron_loop(){
    size_t hours = 0;

    auto voo_price = budget::share_price("VOO");
    std::cout << "Debug: VOO " << voo_price << std::endl;

    while(true){
        using namespace std::chrono_literals;

        std::this_thread::sleep_for(1h);
        ++hours;

        check_for_recurrings();

        if(hours % 72 == 0){
            std::cout << "Refresh the currency cache" << std::endl;
            budget::refresh_currency_cache();
        }
    }
}

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

    std::thread server_thread([](){ start_server(); });
    std::thread cron_thread([](){ start_cron_loop(); });

    server_thread.join();
    cron_thread.join();
}

bool budget::is_server_running(){
    return server_running;
}
