cmake_minimum_required (VERSION 2.8.11)
project (THREDS)

ADD_DEFINITIONS ( -Wall -pedantic -Wextra -Werror --std=c++11 -pg -g )
add_executable (threds main.cpp)
include_directories(googletest/googletest/include/)
ADD_LIBRARY(gtest STATIC IMPORTED)
SET_TARGET_PROPERTIES(gtest PROPERTIES
	IMPORTED_LOCATION  ${CMAKE_SOURCE_DIR}/gtest/libgtest.a)
target_link_libraries(threds gtest pthread)
