cmake_minimum_required(VERSION 3.23)
project(vector)

include_directories(include)

set(CMAKE_CXX_STANDARD 23)

find_package(GTest REQUIRED)

add_executable(test_vector test/test_vector.cpp)
target_link_libraries(test_vector GTest::GTest GTest::Main)

add_test(test_vector test_vector)
