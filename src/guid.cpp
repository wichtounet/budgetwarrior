//=======================================================================
// Copyright (c) 2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
