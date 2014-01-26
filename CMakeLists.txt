cmake_minimum_required(VERSION 2.8.8)

project("budgetwarrior")

#Set the version number
set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(VERSION_PATCH 1)

#Boost is necessary
FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS})

# Activate C++11
add_definitions(-std=c++11)

# Set debug mode options
IF(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    add_definitions(-g)
ENDIF(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")

# Set release mode options
IF(CMAKE_BUILD_TYPE MATCHES "[Re][Ee][Ll][Ee][Aa][Ss][Ee]")
    add_definitions(-g -O2 -march=native)
ENDIF(CMAKE_BUILD_TYPE MATCHES "[Re][Ee][Ll][Ee][Aa][Ss][Ee]")

# Activate specific warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-pedantic -Wuninitialized -Wall -Wextra -Wno-long-long -Winit-self)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions(-pedantic -Wuninitialized -Wsometimes-uninitialized -Wall -Wextra -Wno-long-long -Winit-self -Wdocumentation -Qunused-arguments)
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
