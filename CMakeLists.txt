cmake_minimum_required (VERSION 3.8)
project (budgetwarrier)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(warnings "-Wall -Wextra -Werror")

find_package(OpenSSL REQUIRED)

include_directories(include)
include_directories(cpp-httplib)
include_directories(${OPENSSL_INCLUDE_DIR})

add_subdirectory(src)
