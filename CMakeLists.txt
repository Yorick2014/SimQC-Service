cmake_minimum_required(VERSION 3.16)
project(qkd_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(simqc)
add_subdirectory(app)
