//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DATA_H
#define DATA_H

namespace budget {

template<typename T>
struct data_handler {
    std::size_t next_id;
    std::vector<T> data;
};

} //end of namespace budget

#endif
