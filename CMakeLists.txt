cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

# C++ version.
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-g -Wall -Wextra)

# Dump compile-commands.json.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Create the project.
project(tbd)
set(target "tbd")

# Set up directories.
set(CMAKE_BINARY_DIR ./bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_DIR ./lib)
set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_DIR})

# Set up sources and libraries. */
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/raylib" raylib EXCLUDE_FROM_ALL)

# Build application.
add_executable(${target} ${SOURCES})
target_link_libraries(${target} PUBLIC raylib)
