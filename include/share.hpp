//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <string>

namespace budget {

struct date;
struct money;

money share_price(const std::string& quote);
money share_price(const std::string& quote, budget::date d);

void load_share_price_cache();
void save_share_price_cache();
void prefetch_share_price_cache();

} //end of namespace budget
