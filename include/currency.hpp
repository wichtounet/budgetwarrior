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

double exchange_rate(const std::string& from);
double exchange_rate(const std::string& from, const budget::date& d);
double exchange_rate(const std::string& from, const std::string& to);
double exchange_rate(const std::string& from, const std::string& to, const budget::date& d);

void load_currency_cache();
void save_currency_cache();
void refresh_currency_cache();

} //end of namespace budget
