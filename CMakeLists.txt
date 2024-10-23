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

# Add raylib.
add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/raylib" raylib EXCLUDE_FROM_ALL)

# Add Dear ImGui.
set(imgui_SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui/)
add_library(imgui
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
)
target_include_directories(imgui PUBLIC ${imgui_external_SOURCE_DIR})
target_link_libraries(imgui PUBLIC)

# Add rlImGui.
set(rlImGui_SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/rlImGui/)
add_library(rlImGui
  ${rlImGui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlImGui PUBLIC ${rlImGui_SOURCE_DIR})
target_include_directories(rlImGui PUBLIC ${imgui_SOURCE_DIR})
target_link_libraries(rlImGui PUBLIC imgui raylib)

# Build application.
file(GLOB TBD_SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
add_executable(${target} ${TBD_SOURCES})
target_include_directories(${target} PUBLIC ${rlImGui_SOURCE_DIR})
target_link_libraries(${target} PUBLIC raylib imgui rlImGui)
