cmake_minimum_required(VERSION 3.0)
project(m3_tests)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source and include directories
set(SRC_DIR ../models/m3/source)
set(EXT_DIR ../models/m3/external)
set(BRIDGE_DIR ../models/m3/bridge)

# Add the main m3 library
add_subdirectory(../models/m3/ ${CMAKE_BINARY_DIR}/top)

# Find test source files
set(TEST_SOURCES
    test_main.cc
    test_framework.cc
    m3_test_utils.cc
    base_test_1.cc
    base_test_2.cc
    base_test_3.cc
    base_test_4.cc
    base_test_5.cc
    base_test_6.cc
    base_test_7.cc
    base_test_8.cc
    base_test_9.cc
    base_test_10.cc
    base_test_11.cc
    base_test_12.cc
    base_test_13.cc
)

# Define test executable
add_executable(all_tests ${TEST_SOURCES})

# Link with m3 static library
target_link_libraries(all_tests PRIVATE m3)

# Include m3 headers
target_include_directories(all_tests PRIVATE
    ${SRC_DIR}
    ${EXT_DIR}
    ${BRIDGE_DIR}
    ../core_rtl/Sm2_a/rtl
    .
)
