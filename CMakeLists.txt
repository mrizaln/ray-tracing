cmake_minimum_required(VERSION 3.24)
project(ray-tracing VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON)   # You might want to enable this (CMake 3.24+)

find_package(fmt CONFIG REQUIRED)
find_package(stb CONFIG REQUIRED)
find_package(concurrencpp CONFIG REQUIRED)
find_package(ut CONFIG REQUIRED)

add_executable(main source/main.cpp)
target_include_directories(main PRIVATE source)
target_link_libraries(main PRIVATE fmt::fmt stb::stb concurrencpp::concurrencpp)

add_executable(vec_test test/vec_test.cpp)
target_include_directories(vec_test PRIVATE source)
target_link_libraries(vec_test PRIVATE fmt::fmt Boost::ut)

