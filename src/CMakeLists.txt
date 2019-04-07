cmake_minimum_required(VERSION 3.8)

file(GLOB SOURCES "*.cpp")
file(GLOB PAGES "pages/*.cpp")
file(GLOB API "api/*.cpp")

add_executable(budget ${SOURCES} ${PAGES} ${API})

target_link_libraries(budget OpenSSL::SSL)
