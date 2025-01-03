cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

# Compiler.
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

# C++ version.
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-g -Wall -Wextra -Wno-c99-designator)

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
if (DEFINED UNIX AND NOT APPLE)
  set(GLFW_BUILD_WAYLAND OFF)
  set(GLFW_BUILD_X11 ON)
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/raylib" raylib EXCLUDE_FROM_ALL)

# Add raygui.
set(raygui_SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/raygui/src/)

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
target_include_directories(${target} PUBLIC ${rlImGui_SOURCE_DIR} ${raygui_SOURCE_DIR})
target_link_libraries(${target} PUBLIC raylib imgui rlImGui)

# Add building for web with Emscripten.
if (EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set_target_properties(${target} PROPERTIES
    LINK_FLAGS "-s USE_GLFW=3 -s ASYNCIFY -s ASSERTIONS -s TOTAL_STACK=64MB -s INITIAL_MEMORY=128MB -DPLATFORM_WEB -O3 -o landing.html --preload-file assets")
endif()

# Install assets.
add_custom_target(copy_assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/bin/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(${target} copy_assets)
