cmake_minimum_required(VERSION 3.10)
project(BresenhC)

set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

include_directories(opt/homebrew/include ${PROJECT_SOURCE_DIR}/include)
link_directories(opt/homebrew/lib)

find_package(SDL3 REQUIRED)

add_executable(BresenhC ${SOURCES})

target_include_directories(BresenhC PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(BresenhC PRIVATE SDL3::SDL3)