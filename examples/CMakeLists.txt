cmake_minimum_required(VERSION 3.17)

add_executable(WFCExamples
    usage_example.cpp
)

target_link_libraries(WFCExamples PRIVATE WaveFunctionCollapse)
