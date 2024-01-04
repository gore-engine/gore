cmake_minimum_required(VERSION 3.20)

project(RealtimeMath)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(RealtimeMath INTERFACE)
target_include_directories(RealtimeMath INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../)