//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000
#else
#include <uuid/uuid.h>
#endif

#include "guid.hpp"

std::string budget::generate_guid(){
#ifdef _WIN32
    UUID uuid;
    UuidCreate(&uuid);
    char *uuid_string;
    UuidToStringA(&uuid, (RPC_CSTR*)&uuid_string);
    std::string ret(uuid_string);
    RpcStringFreeA((RPC_CSTR*)&uuid_string);
    return ret;
#else
    uuid_t uuid;
    char uuid_string[37];

    uuid_generate(uuid);
    uuid_unparse_upper(uuid, uuid_string);
#endif
    return std::string(uuid_string);
}
