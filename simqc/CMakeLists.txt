file(GLOB SIMQC_SOURCES src/*.cpp)
add_library(qkd_lib ${SIMQC_SOURCES})

target_include_directories(qkd_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)