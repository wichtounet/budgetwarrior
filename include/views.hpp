//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <ranges>

namespace budget {

// We define views without parameters as an adaptor to avoid having to use ()

namespace detail {

struct share_based_only_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, share_based_only_adaptor) {
        return std::forward<R>(r) | std::views::filter([](const auto& asset) { return asset.share_based; });
    }
};

struct to_name_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, to_name_adaptor) {
	    return std::forward<R>(r) | std::views::transform([](auto & element) { return element.name; });
    }
};

struct to_date_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, to_date_adaptor) {
        if constexpr (std::is_same_v<std::remove_cv_t<std::ranges::range_value_t<R>>, budget::asset_value>) {
            return std::forward<R>(r) | std::views::transform([](auto & element) { return element.set_date; });
        } else {
            return std::forward<R>(r) | std::views::transform([](auto & element) { return element.date; });
        }
    }
};

struct only_open_ended_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, only_open_ended_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & account) { return account.until == budget::date(2099,12,31); });
    }
};

struct not_open_ended_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, not_open_ended_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & account) { return account.until != budget::date(2099,12,31); });
    }
};

struct active_today_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, active_today_adaptor) {
        auto today = budget::local_day();
        return std::forward<R>(r) | std::views::filter([today] (const auto & account) { return account.since < today && account.until > today; });
    }
};

struct not_liability_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, not_liability_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & asset) { return !asset.liability; });
    }
};

struct is_desired_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, is_desired_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & asset) { return asset.name == "DESIRED" && asset.currency == "DESIRED"; });
    }
};

} // namespace detail

inline auto filter_by_id(size_t id) {
    return std::views::filter([id] (const auto & element) { return element.id == id; });
}

inline auto not_id(size_t id) {
    return std::views::filter([id] (const auto & element) { return element.id != id; });
}

inline auto filter_by_account(size_t id) {
    return std::views::filter([id] (const auto & expense) { return expense.account == id; });
}

inline auto filter_by_asset(size_t id) {
    return std::views::filter([id] (const auto & share) { return share.asset_id == id; });
}

inline auto filter_by_name(const std::string & name) {
    return std::views::filter([&name] (const auto & account) { return account.name == name; });
}

inline auto active_at_date(budget::date date) {
    return std::views::filter([date] (const auto & account) { return account.since < date && account.until > date; });
}

inline auto since(budget::date since) {
    return std::views::filter([since] (const auto & expense) { return expense.date >= since; });
}

inline constexpr detail::active_today_adaptor active_today;
inline constexpr detail::only_open_ended_adaptor only_open_ended;
inline constexpr detail::not_open_ended_adaptor not_open_ended;
inline constexpr detail::share_based_only_adaptor share_based_only;
inline constexpr detail::to_name_adaptor to_name;
inline constexpr detail::to_date_adaptor to_date;
inline constexpr detail::not_liability_adaptor not_liability;
inline constexpr detail::is_desired_adaptor is_desired;

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
