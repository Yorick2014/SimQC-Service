file(GLOB_RECURSE APP_SOURCES
    src/*.cpp
)

add_executable(qkd_api ${APP_SOURCES})

target_include_directories(qkd_api PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/simqc/include
)

target_link_libraries(qkd_api PRIVATE qkd_lib pthread)
