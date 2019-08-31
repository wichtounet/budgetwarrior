//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

namespace budget {

template <typename Iterator, typename Filter>
struct filter_iterator {
    using value_type = typename Iterator::value_type;

    filter_iterator(Iterator first, Iterator last, Filter filter)
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
    Iterator first;
    Iterator last;
    Filter filter;
};

template<typename Iterator, typename Filter>
struct filter_view {
    filter_view(Iterator first, Iterator last, Filter filter) : first(first), last(last), filter(filter) {}

    auto begin() const {
        return filter_iterator<Iterator, Filter>(first, last, filter);
    }

    auto end() const {
        return filter_iterator<Iterator, Filter>(last, last, filter);
    }

    auto to_vector() const {
        std::vector<typename Iterator::value_type> copy;

        auto it  = this->begin();
        auto end = this->end();

        while (it != end) {
            copy.push_back(*it);
            ++it;
        }

        return copy;
    }

private:
    Iterator first;
    Iterator last;
    Filter filter;
};

template <typename Iterator, typename Filter>
auto make_filter_view(Iterator first, Iterator last, Filter filter){
    return filter_view<Iterator, Filter>(first, last, filter);
}

} //end of namespace budget
