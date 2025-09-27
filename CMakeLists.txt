cmake_minimum_required(VERSION 3.14)
project(QKD_Simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Программа
add_executable(qc_service src/main.cpp src/laser.cpp)

target_include_directories(qc_service PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Линкуем библиотеку
# target_link_libraries(qkd_simulator PRIVATE pulse)

# Компиляторные флаги
# target_compile_options(qkd_simulator PRIVATE -Wall -Wextra -Wpedantic)
