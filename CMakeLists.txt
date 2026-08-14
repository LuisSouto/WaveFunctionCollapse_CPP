cmake_minimum_required(VERSION 3.17)
project(WaveFunctionCollapse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # needed for clangd

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Option to build the shared library without examples
option(WFC_BUILD_EXAMPLES "Build examples" ON)

add_library(WaveFunctionCollapse SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/src/adjacency_data.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/overlapping_patterns.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sprite_holder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sprite_reader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/stb_image.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/wfc_core.cpp"
)

target_include_directories(WaveFunctionCollapse
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(WaveFunctionCollapse PRIVATE
    -O3
    -ffp-contract=fast
    -g
    -march=native
    -mtune=native
    -Wall
    -Wextra
    -Wpedantic
)

if(WFC_BUILD_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
        add_subdirectory(examples)
    endif()
endif()
