//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <functional>

namespace budget {

template <typename Type>
struct filter_iterator {
    using value_type    = Type;
    using iterator_type = typename std::vector<value_type>::const_iterator;

    template <typename Filter>
    filter_iterator(iterator_type first, iterator_type last, Filter filter)
            : first(first), last(last), filter(filter) {
        while(this->first != this->last && !this->filter(*this->first)){
            ++this->first;
        }
    }

    filter_iterator& operator++() {
        if (first == last) {
            return *this;
        }

        do {
            ++first;
        } while (first != last && !filter(*first));

        return *this;
    }

    bool operator==(const filter_iterator& rhs) {
        return first == rhs.first;
    }

    bool operator!=(const filter_iterator& rhs) {
        return first != rhs.first;
    }

    decltype(auto) operator*() {
        return *first;
    }

    decltype(auto) operator*() const {
        return *first;
    }

    decltype(auto) operator->() {
        return &*first;
    }

    decltype(auto) operator->() const {
        return &*first;
    }

private:
    iterator_type first;
    iterator_type last;
    std::function<bool(const Type&)> filter;
};

template<typename Type>
struct filter_view {
    template <typename Filter>
    filter_view(const std::vector<Type> & container, Filter filter) : container(container), filter(filter) {}

    auto begin() const {
        return filter_iterator<Type>(container.begin(), container.end(), filter);
    }

    auto end() const {
        return filter_iterator<Type>(container.end(), container.end(), filter);
    }

    auto to_vector() const {
        std::vector<Type> copy;

        auto it  = this->begin();
        auto end = this->end();

        while (it != end) {
            copy.push_back(*it);
            ++it;
        }

        return copy;
    }

private:
    const std::vector<Type> & container;
    std::function<bool (const Type &)> filter;
};

template <typename Type, typename Filter>
filter_view<Type> make_filter_view(const std::vector<Type> & container, Filter filter){
    return filter_view<Type>(container, filter);
}

} //end of namespace budget
