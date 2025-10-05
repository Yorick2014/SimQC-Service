cmake_minimum_required(VERSION 3.14)
project(QKD_Simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(qc_service src/main.cpp src/laser.cpp src/sequence_gen.cpp)

target_include_directories(qc_service PRIVATE ${CMAKE_SOURCE_DIR}/include)
