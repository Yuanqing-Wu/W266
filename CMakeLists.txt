cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(W266 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_DIR ${CMAKE_SOURCE_DIR}/Common)
set(DECODER_DIR ${CMAKE_SOURCE_DIR}/Decoder)

file(GLOB COMMON_SOURCES ${COMMON_DIR}/*.cpp)
file(GLOB DECODER_SOURCES ${DECODER_DIR}/*.cpp)

file(GLOB COMMON_HEADERS ${COMMON_DIR}/*.h)
file(GLOB DECODER_HEADERS ${DECODER_DIR}/*.h)

#include_directories(${COMMON_DIR} ${DECODER_DIR})\
include_directories(${CMAKE_SOURCE_DIR})

add_library(decoder STATIC ${DECODER_SOURCES})

add_executable(dec App/Dec.cpp
    ${APP_SOURCES}
    ${COMMON_SOURCES}
    ${APP_HEADERS}
    ${COMMON_HEADERS}
)

target_link_libraries(dec decoder)