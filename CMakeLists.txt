cmake_minimum_required(VERSION 3.16)
project(graham_scan_rbt LANGUAGES CXX)

add_executable(hull src/main.cpp src/rbt.cpp)
target_include_directories(hull PRIVATE include)
target_compile_features(hull PRIVATE cxx_std_17)
target_compile_options(hull PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME hull_self_test COMMAND hull --self-test)
