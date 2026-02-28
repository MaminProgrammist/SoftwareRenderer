cmake_minimum_required(VERSION 3.16)
project(SoftwareRenderer)

set(CMAKE_CXX_STANDARD 20)

set(SDL3_DIR "${CMAKE_SOURCE_DIR}/external/SDL3/lib/cmake/SDL3")
find_package(SDL3 REQUIRED CONFIG)

add_executable(SoftwareRenderer src/main.cpp)

target_link_libraries(SoftwareRenderer PRIVATE SDL3::SDL3)

add_custom_command(TARGET SoftwareRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/external/SDL3/bin/SDL3.dll"
        $<TARGET_FILE_DIR:SoftwareRenderer>)