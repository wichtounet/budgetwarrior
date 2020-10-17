//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <mutex>

#include "server.hpp"

namespace budget {

struct server_lock {
    void lock() {
        if (is_server_running()) {
            mutex_lock.lock();
        }
    }

    void unlock() {
        if (is_server_running()) {
            mutex_lock.unlock();
        }
    }

private:
    std::mutex mutex_lock;
};

using server_lock_guard = std::lock_guard<server_lock>;

} //end of namespace budget
