//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <uuid/uuid.h>

#include "guid.hpp"

std::string budget::generate_guid(){
    uuid_t uuid;
    char uuid_string[37];

    uuid_generate(uuid);
    uuid_unparse_upper(uuid, uuid_string);

    return std::string(uuid_string);
}
