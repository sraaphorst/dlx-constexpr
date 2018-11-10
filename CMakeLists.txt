# CMakeLists.txt
# By Sebastian Raaphorst, 2018

cmake_minimum_required(VERSION 3.12)
project(dlx_constexpr_top)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-narrowing -std=c++17 -ftemplate-depth=10000 -fconstexpr-depth=2000")

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/test")
add_subdirectory(include)
add_subdirectory(test)
