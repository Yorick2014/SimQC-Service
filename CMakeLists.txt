cmake_minimum_required(VERSION 3.14)
project(QKD_Simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(qkd_lib
    src/bb84.cpp
    src/laser.cpp
    src/sequence_generator.cpp
    src/modulator.cpp
)

target_include_directories(qkd_lib
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

add_executable(simple-bb84 example/simple-bb84.cpp)
target_link_libraries(simple-bb84 PRIVATE qkd_lib)
