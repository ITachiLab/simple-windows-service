cmake_minimum_required(VERSION 3.12)
project(foo_service)

set(CMAKE_CXX_STANDARD 14)

add_executable(foo_service service.cpp)