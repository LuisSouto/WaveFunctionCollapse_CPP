cmake_minimum_required(VERSION 3.17)
project(WaveFunctionCollapse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # needed for clangd

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Option to build the shared library without examples
option(WFC_BUILD_EXAMPLES "Build example applications" ON)
option(WFC_BUILD_TESTS "Build unit tests" ON)

file(GLOB_RECURSE WFC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(WaveFunctionCollapse SHARED
    ${WFC_SOURCES}
)

target_include_directories(WaveFunctionCollapse
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(WFC_BUILD_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
        add_subdirectory(examples)
    endif()
endif()

if(WFC_BUILD_TESTS)
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()
