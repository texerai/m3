cmake_minimum_required(VERSION 3.10)
project(M3Tests LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_MODE Debug)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/bridges/bridge_boom
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/legacy_memmodel
)

# Create shared test library with common utilities
add_library(test_common STATIC
    common/m3_test_utils.cc
    common/test_framework.cc
)

# Link test_common with the main M3 library
target_link_libraries(test_common PRIVATE m3)

# Legacy memory model tests
set(LEGACY_TEST_SOURCES
    legacy_memmodel/base_test_1.cc
    legacy_memmodel/base_test_2.cc
    legacy_memmodel/base_test_3.cc
    legacy_memmodel/base_test_4.cc
    legacy_memmodel/base_test_5.cc
    legacy_memmodel/base_test_6.cc
    legacy_memmodel/base_test_8.cc
    legacy_memmodel/base_test_9.cc
    legacy_memmodel/base_test_10.cc
    legacy_memmodel/base_test_11.cc
    legacy_memmodel/base_test_12.cc
    legacy_memmodel/base_test_13.cc
)

# Create legacy test executable
add_executable(legacy_memmodel_tests
    legacy_memmodel/test_main.cc
    ${LEGACY_TEST_SOURCES}
)

# Modern tests
add_executable(core_test core_test.cpp)
add_executable(data_test data_test.cpp)

add_executable(bridge_boom_test
    bridge_boom_test.cpp
    test_trace_reader/test_trace_reader.cpp
    ${CMAKE_SOURCE_DIR}/src/bridges/bridge_boom/bridge_boom.cpp
)

# Link legacy tests with shared test library
target_link_libraries(legacy_memmodel_tests PRIVATE test_common)

# Link modern tests with the main library
target_link_libraries(core_test PRIVATE m3)
target_link_libraries(data_test PRIVATE m3)
target_link_libraries(bridge_boom_test PRIVATE m3)

# Enable tests
enable_testing()

# Add all tests
add_test(NAME legacy_memmodel_tests COMMAND legacy_memmodel_tests)
add_test(NAME core_test COMMAND core_test)
add_test(NAME data_test COMMAND data_test)
add_test(NAME bridge_boom_test COMMAND bridge_boom_test)
