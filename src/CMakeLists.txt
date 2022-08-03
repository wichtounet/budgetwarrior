cmake_minimum_required(VERSION 3.8)

file(GLOB SOURCES "*.cpp")
file(GLOB PAGES "pages/*.cpp")
file(GLOB API "api/*.cpp")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
pkg_search_module(UUID REQUIRED uuid)

add_executable(budget ${SOURCES} ${PAGES} ${API})
target_include_directories(budget PUBLIC ${UUID_INCLUDE_DIRS})
target_link_libraries(budget OpenSSL::SSL Threads::Threads ${CMAKE_DL_LIBS} ${UUID_LIBRARIES})
install(TARGETS budget DESTINATION bin/)
