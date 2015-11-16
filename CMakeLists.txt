cmake_minimum_required (VERSION 2.8.11)
project (THREDS)

find_package(Threads REQUIRED)
# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
gtest
URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
# Disable install step
INSTALL_COMMAND ""
)

add_library(gtestl IMPORTED STATIC GLOBAL)

# Set gtest properties
ExternalProject_Get_Property(gtest source_dir binary_dir)
set_target_properties(gtestl PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

# I couldn't make it work with INTERFACE_INCLUDE_DIRECTORIES
include_directories("${source_dir}/include")


ADD_DEFINITIONS ( -Wall -pedantic -Wextra -Werror --std=c++11 -pg -g )
add_executable (threds main.cpp)
target_link_libraries(threds    gtestl pthread)
