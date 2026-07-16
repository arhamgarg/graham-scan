cmake_minimum_required(VERSION 3.16)
project(graham_scan_avl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(hull
    src/main.cpp
    src/avl.cpp
    src/rbt.cpp
    src/arr.cpp
)

target_include_directories(hull PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(hull PRIVATE -O3 -Wall -Wextra)

enable_testing()
add_test(NAME self_test COMMAND hull --self-test)
