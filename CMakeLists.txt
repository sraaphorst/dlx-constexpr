# CMakeLists.txt
# By Sebastian Raaphorst, 2018

cmake_minimum_required(VERSION 3.12)
project(dlx_constexpr)

set(CMAKE_CXX_STANDARD 17)

add_executable(dlx_constexpr dlx_contexpr.cpp)