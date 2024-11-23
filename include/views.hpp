//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <ranges>

#include "date.hpp"
#include "assets.hpp"
#include "expenses.hpp"
#include "liabilities.hpp"

namespace ranges = std::ranges;

namespace budget {

std::string get_account_name(size_t id);

// We define views without parameters as an adaptor to avoid having to use ()

namespace detail {

struct monthly_only_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, monthly_only_adaptor) {
        return std::forward<R>(r) | std::views::filter([](const auto& objective) { return objective.type == "monthly"; });
    }
};

struct yearly_only_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, yearly_only_adaptor) {
        return std::forward<R>(r) | std::views::filter([](const auto& objective) { return objective.type == "yearly"; });
    }
};

struct temporary_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, temporary_adaptor) {
        return std::forward<R>(r) | std::views::filter([](const auto& expense) { return expense.temporary; });
    }
};

struct persistent_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, persistent_adaptor) {
        return std::forward<R>(r) | std::views::filter([](const auto& expense) { return !expense.temporary; });
    }
};

struct share_based_only_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, share_based_only_adaptor) {
        return std::forward<R>(r) | std::views::filter([](const auto& asset) { return asset.share_based; });
    }
};

struct not_share_based_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, not_share_based_adaptor) {
        return std::forward<R>(r) | std::views::filter([](const auto& asset) { return !asset.share_based; });
    }
};

struct to_name_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, to_name_adaptor) {
	    return std::forward<R>(r) | std::views::transform([](auto & element) { return element.name; });
    }
};

struct to_amount_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, to_amount_adaptor) {
	    return std::forward<R>(r) | std::views::transform([](auto & element) { return element.amount; });
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

struct to_month_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, to_month_adaptor) {
        if constexpr (std::is_same_v<std::remove_cv_t<std::ranges::range_value_t<R>>, budget::date>) {
            return std::forward<R>(r) | std::views::transform([](const auto & date) { return date.month(); });
        } else {
            return std::forward<R>(r) | std::views::transform([](const auto & element) { return element.date.month(); });
        }
    }
};

struct to_year_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, to_year_adaptor) {
        if constexpr (std::is_same_v<std::remove_cv_t<std::ranges::range_value_t<R>>, budget::date>) {
            return std::forward<R>(r) | std::views::transform([](const auto & date) { return date.year(); });
        } else {
            return std::forward<R>(r) | std::views::transform([](const auto & element) { return element.date.year(); });
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

struct is_active_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, is_active_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & asset) { return asset.active; });
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

struct liability_only_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, liability_only_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & asset) { return asset.liability; });
    }
};

struct is_user_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, is_user_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & asset) { return asset.name != "DESIRED" && asset.currency != "DESIRED"; });
    }
};

struct is_desired_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, is_desired_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & asset) { return asset.name == "DESIRED" && asset.currency == "DESIRED"; });
    }
};

struct is_portfolio_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, is_portfolio_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & element) { return element.portfolio; });
    }
};

struct is_fi_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, is_fi_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & element) { return element.is_fi(); });
    }
};

struct is_cash_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, is_cash_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & element) { return element.is_cash(); });
    }
};

struct not_zero_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, not_zero_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & pair) { return !pair.second.zero(); });
    }
};

struct not_paid_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, not_paid_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & element) { return !element.paid; });
    }
};

struct paid_only_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, paid_only_adaptor) {
        return std::forward<R>(r) | std::views::filter([] (const auto & element) { return element.paid; });
    }
};

struct not_template_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, not_template_adaptor) {
        return std::forward<R>(r) | std::views::filter([] <typename T> (const T& element) {
                   if constexpr (std::is_same_v<T, budget::date>) {
                       return element != TEMPLATE_DATE;
                   } else {
                       return element.date != TEMPLATE_DATE;
                   }
               });
    }
};

struct template_only_adaptor {
    template <std::ranges::range R>
    friend auto operator|(R&& r, template_only_adaptor) {
        return std::forward<R>(r) | std::views::filter([] <typename T>(const T& element) {
                   if constexpr (std::is_same_v<T, budget::date>) {
                       return element == TEMPLATE_DATE;
                   } else {
                       return element.date == TEMPLATE_DATE;
                   }
               });
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

inline auto filter_by_account_name(std::string_view name) {
    return std::views::filter([name] (const auto & expense) { return get_account_name(expense.account) == name; });
}

inline auto filter_by_asset(size_t id) {
    return std::views::filter([id] (const auto & share) { return share.asset_id == id; });
}

inline auto filter_by_type(std::string_view type) {
    return std::views::filter([type] (const auto & element) { return element.type == type; });
}

inline auto filter_by_currency(std::string_view currency) {
    return std::views::filter([currency] (const auto & element) { return element.currency == currency; });
}

inline auto filter_by_name(std::string_view name) {
    return std::views::filter([name] (const auto & account) { return account.name == name; });
}

inline auto filter_by_amount(budget::money amount) {
    return std::views::filter([amount] (const auto & element) { return element.amount == amount; });
}

inline auto filter_by_ticker(std::string_view ticker) {
    return std::views::filter([ticker] (const auto & account) { return account.ticker == ticker; });
}

inline auto filter_by_year(budget::year year) {
    return std::views::filter([year] <typename T> (const T & element) -> bool {
        if constexpr (std::is_same_v<T, budget::date>) {
            return element.year() == year;
        } else {
            return element.date.year() == year;
        }
    });
}

inline auto filter_by_month(budget::month month) {
    return std::views::filter([month] <typename T> (const T & element) -> bool {
        if constexpr (std::is_same_v<T, budget::date>) {
            return element.month() == month;
        } else {
            return element.date.month() == month;
        }
    });
}

inline auto filter_by_date(budget::year year, budget::month month) {
    return std::views::filter([year, month] <typename T> (const T & element) -> bool {
        if constexpr (std::is_same_v<T, budget::date>) {
            return element.year() == year && element.month() == month;
        } else {
            return element.date.year() == year && element.date.month() == month;
        }
    });
}

inline auto between(budget::month sm, budget::month month) {
    return std::views::filter([sm, month] <typename T> (const T & element) -> bool {
        if constexpr (std::is_same_v<T, budget::date>) {
            return element.month() >= sm && element.month() <= month;
        } else {
            return element.date.month() >= sm && element.date.month() <= month;
        }
    });
}

inline auto active_at_date(budget::date date) {
    return std::views::filter([date] (const auto & account) { return account.since < date && account.until > date; });
}

inline auto since(budget::date since) {
    return std::views::filter([since] (const auto & expense) { return expense.date >= since; });
}

inline auto expand_value(data_cache& cache) {
    return std::views::transform([&cache]<typename T> (const T& asset) {
        if constexpr (std::is_same_v<T, budget::liability>) {
            auto amount = get_liability_value(asset, cache);
            return std::make_pair(asset, amount);
        } else {
            auto amount = get_asset_value(asset, cache);
            return std::make_pair(asset, amount);
        }
    });
}

inline auto expand_value_conv(data_cache& cache, budget::date d = budget::local_day()) {
    return std::views::transform([&cache, d]<typename T> (const T& asset) {
        if constexpr (std::is_same_v<T, budget::liability>) {
            auto amount = get_liability_value_conv(asset, d, cache);
            return std::make_pair(asset, amount);
        } else {
            auto amount = get_asset_value_conv(asset, d, cache);
            return std::make_pair(asset, amount);
        }
    });
}

inline auto to_value(data_cache& cache) {
    return std::views::transform([&cache]<typename T> (const T& asset) {
        if constexpr (std::is_same_v<T, budget::liability>) {
            return get_liability_value(asset, cache);
        } else {
            return get_asset_value(asset, cache);
        }
    });
}

inline auto to_value_conv(data_cache& cache, budget::date d = budget::local_day()) {
    return std::views::transform([&cache, d]<typename T> (const T& asset) {
        if constexpr (std::is_same_v<T, budget::liability>) {
            return get_liability_value_conv(asset, d, cache);
        } else {
            return get_asset_value_conv(asset, d, cache);
        }
    });
}

inline constexpr detail::is_active_adaptor is_active;
inline constexpr detail::active_today_adaptor active_today;
inline constexpr detail::only_open_ended_adaptor only_open_ended;
inline constexpr detail::not_open_ended_adaptor not_open_ended;
inline constexpr detail::monthly_only_adaptor monthly_only;
inline constexpr detail::yearly_only_adaptor yearly_only;
inline constexpr detail::share_based_only_adaptor share_based_only;
inline constexpr detail::not_share_based_adaptor not_share_based;
inline constexpr detail::temporary_adaptor temporary;
inline constexpr detail::persistent_adaptor persistent;
inline constexpr detail::to_name_adaptor to_name;
inline constexpr detail::to_amount_adaptor to_amount;
inline constexpr detail::to_date_adaptor to_date;
inline constexpr detail::to_month_adaptor to_month;
inline constexpr detail::to_year_adaptor to_year;
inline constexpr detail::liability_only_adaptor liability_only;
inline constexpr detail::not_liability_adaptor not_liability;
inline constexpr detail::is_desired_adaptor is_desired;
inline constexpr detail::is_user_adaptor is_user;
inline constexpr detail::is_portfolio_adaptor is_portfolio;
inline constexpr detail::is_fi_adaptor is_fi;
inline constexpr detail::is_cash_adaptor is_cash;
inline constexpr detail::not_zero_adaptor not_zero;
inline constexpr detail::paid_only_adaptor paid_only;
inline constexpr detail::not_paid_adaptor not_paid;
inline constexpr detail::template_only_adaptor template_only;
inline constexpr detail::not_template_adaptor not_template;

// Syntactic sugar around fold_left
template <std::ranges::range R>
auto fold_left_auto(R&& r) {
    using type = std::ranges::range_value_t<R>;
#ifdef __clang__
    type value{};
    for (const type & v : r) {
        value += v;
    }
    return value;
#else
    // On GCC; we can simply use fold left
    return std::ranges::fold_left(std::forward<R>(r), type{}, std::plus<type>());
#endif
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

template <std::ranges::range R>
auto min_with_default(R&& r, std::ranges::range_value_t<R> def) {
    if (r) {
        return std::ranges::min(std::forward<R>(r));
    }

    return def;
}

// Stupid clang does not support std::ranges::contains

template <std::ranges::range R>
bool range_contains(const R& r, const std::ranges::range_value_t<R> & value) {
#ifdef __clang__
    return std::ranges::find(std::ranges::begin(r), std::ranges::end(r), value) == std::ranges::end(r);
#else
    // On GCC; we can simply use the algorithm
    return std::ranges::contains(r, value);
#endif
}

} //end of namespace budget
