//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>
#include <thread>
#include <thread>
#include <condition_variable>

#include "cpp_utils/assert.hpp"

#include "server.hpp"
#include "expenses.hpp"
#include "earnings.hpp"
#include "accounts.hpp"
#include "incomes.hpp"
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

httplib::Server * server_ptr = nullptr;
volatile bool cron = true;

std::mutex lock;
std::condition_variable cv;

void server_signal_handler(int signum) {
    std::cout << "INFO: Received signal (" << signum << ")" << std::endl;

    cron = false;

    if (server_ptr) {
        server_ptr->stop();
    }

    cv.notify_all();
}

void install_signal_handler() {
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = server_signal_handler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    std::cout << "INFO: Installed the signal handler" << std::endl;
}

bool start_server(){
    std::cout << "INFO: Started the server thread" << std::endl;

    httplib::Server server;

    load_pages(server);
    load_api(server);

    install_signal_handler();

    auto port = get_server_port();
    auto listen = get_server_listen();
    server_ptr = &server;

    // Listen
    std::cout << "INFO: Server is starting to listen on " << listen << ':' << port << std::endl;
    if (!server.listen(listen.c_str(), port)) {
        std::cerr << "INFO: Server failed to start" << std::endl;
        return false;
    }

    std::cout << "INFO: Server has exited normally" << std::endl;
    return true;
}

void start_cron_loop(){
    std::cout << "INFO: Started the cron thread" << std::endl;
    size_t hours = 0;

    while(cron){
        using namespace std::chrono_literals;

        {
            std::unique_lock<std::mutex> lk(lock);
            cv.wait_for(lk, 1h);

            if (!cron) {
                break;
            }
        }

        ++hours;

        check_for_recurrings();

        // We save the cache once per day
        if (hours % 24 == 0) {
            save_currency_cache();
            save_share_price_cache();
        }

        // Every four hours, we refresh the currency cache
        // Only current day rates are refreshed
        if (hours % 4 == 0) {
            std::cout << "Refresh the currency cache" << std::endl;
            budget::refresh_currency_cache();
        }

        // Every hour, we try to prefetch value for new days
        std::cout << "Prefetch the share cache" << std::endl;
        budget::prefetch_share_price_cache();
    }

    std::cout << "INFO: Cron Thread has exited" << std::endl;
}

} //end of anonymous namespace

void budget::set_server_running(){
    // Indicates to the system that it's running in server mode
    server_running = true;
}

void budget::server_module::load(){
    load_accounts();
    load_incomes();
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

    std::cout << "Starting the threads" << std::endl;

    volatile bool success = false;
    std::thread server_thread([&success](){ success = start_server(); });
    std::thread cron_thread([](){ start_cron_loop(); });

    server_thread.join();

    if (!success) {
        cron = false;
    }

    cron_thread.join();
}

bool budget::is_server_running(){
    return server_running;
}
