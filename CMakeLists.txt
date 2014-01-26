cmake_minimum_required(VERSION 2.8.8)

project("budgetwarrior")

#Set the version number
set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(VERSION_PATCH 1)

#Boost is necessary
FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS})

# Activate warnings and C++11 support
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS                 "-pipe -pthread -std=c++11")
    set(CMAKE_EXE_LINKER_FLAGS          "")

    set(CMAKE_CXX_FLAGS_DEBUG           "-g -pedantic -Wuninitialized -Wall -Wextra -Wno-long-long -Winit-self -fstack-protector-all")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG    "")

    set(CMAKE_CXX_FLAGS_RELEASE         "-g -DLOGGING_DISABLE -DNDEBUG -O3 -fomit-frame-pointer -march=native")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE  "")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS           "-std=c++11 -Qunused-arguments")
    set(CMAKE_CXX_FLAGS_DEBUG     "-g -pedantic -Wuninitialized -Wsometimes-uninitialized -Wall -Wextra -Wno-long-long -Winit-self -Wdocumentation")
    set(CMAKE_CXX_FLAGS_RELEASE   "-DLOGGING_DISABLE -DNDEBUG -O3 -march=native")
else()
    MESSAGE(FATAL_ERROR "ERROR: budgetwarrior can only be compiled on gcc and clang for now")
endif()

# Add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
        WORKING_DIRECTORY doc
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

# Add a target to inspect the whole code with cppcheck
add_custom_target(cppcheck cppcheck --platform=unix64 --enable=all --std=c++11 -I ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Analysing the code with cppcheck" VERBATIM
)

# Add a custom target to gather git stats
add_custom_target(gitstats gitstats ${CMAKE_CURRENT_SOURCE_DIR} stats
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Gathering git stats" VERBATIM
)

# The executable should go to the bin directory
set(EXECUTABLE_OUTPUT_PATH bin)

# All the headers are in the include directory
include_directories(include)

# Create a pseudo library grouping all object files

file(
	GLOB_RECURSE
	source_files
	src/*.cpp
)

# Create the budget executable

add_executable(budget ${source_files})
target_link_libraries(budget uuid)
target_link_libraries(budget boost_system)
target_link_libraries(budget boost_filesystem)
target_link_libraries(budget boost_date_time)

# Specifications for the installation of budgetwarrior

INSTALL(TARGETS budget
    RUNTIME DESTINATION bin
)
