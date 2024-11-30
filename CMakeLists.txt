cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(W266 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dec app/dec.cpp)