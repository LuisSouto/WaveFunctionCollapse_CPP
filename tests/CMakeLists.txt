include(FetchContent)

# Fetch Google Test from GitHub
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Coverage option
option(WFC_ENABLE_COVERAGE "Enable code coverage reporting" OFF)

# Add test executable
add_executable(wfc_tests
    sprite_holder_test.cpp
    overlapping_patterns_test.cpp
)

# Link against the main library and Google Test
target_link_libraries(wfc_tests
    PRIVATE
        WaveFunctionCollapse
        gtest_main
        gtest
)

# Enable coverage flags if requested
if(WFC_ENABLE_COVERAGE)
    target_compile_options(wfc_tests PRIVATE --coverage)
    target_link_options(wfc_tests PRIVATE --coverage)
    target_compile_options(WaveFunctionCollapse PRIVATE --coverage)
    target_link_options(WaveFunctionCollapse PRIVATE --coverage)
endif()

# Add tests to ctest
add_test(NAME WFCTests COMMAND wfc_tests)
