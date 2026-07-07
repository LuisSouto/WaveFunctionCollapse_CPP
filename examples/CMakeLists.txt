cmake_minimum_required(VERSION 3.17)

add_executable(WFCSimpleExample
    simple_example.cpp
)

target_link_libraries(WFCSimpleExample PRIVATE WaveFunctionCollapse)
