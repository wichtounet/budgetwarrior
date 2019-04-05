cmake_minimum_required (VERSION 3.8)
project (budgetwarrier)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(warnings "-Wall -Wextra -Werror")


include_directories(include)
include_directories(cpp-httplib)

find_package(OpenSSL REQUIRED)
#if(OPENSSL_FOUND)
#set(OPENSSL_USE_STATIC_LIBS TRUE)
#endif()

#if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
#endif()

include_directories(${OPENSSL_INCLUDE_DIR})

add_subdirectory(src)





