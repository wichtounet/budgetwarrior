//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <ranges>

namespace budget {

inline auto filter_by_account(size_t account_id) {
    return std::views::filter([&account_id] (const auto & expense) { return expense.account == account_id; });
}

inline auto filter_by_name(const std::string & name) {
    return std::views::filter([&name] (const auto & account) { return account.name == name; });
}

inline auto only_open_ended() {
    return std::views::filter([] (const auto & account) { return account.until == budget::date(2099,12,31); });
}

inline auto not_open_ended() {
    return std::views::filter([] (const auto & account) { return account.until != budget::date(2099,12,31); });
}

inline auto active_today() {
    auto today = budget::local_day();
    return std::views::filter([today] (const auto & account) { return account.since < today && account.until > today; });
}

inline auto active_at_date(budget::date date) {
    return std::views::filter([date] (const auto & account) { return account.since < date && account.until > date; });
}

inline auto since(budget::date since) {
    return std::views::filter([since] (const auto & expense) { return expense.date >= since; });
}

// TODO(C+23) In the future, we can simply ranges::to<std::vector> but it is not yet implemented with GCC

template <std::ranges::range R>
auto to_vector(R&& r) {
    std::vector<std::ranges::range_value_t<R>> v;

    // if we can get a size, reserve that much
    if constexpr (requires { std::ranges::size(r); }) {
        v.reserve(std::ranges::size(r));
    }

    for (auto&& e : r) {
        v.emplace_back(static_cast<decltype(e)&&>(e));
    }

    return v;
}

} //end of namespace budget
