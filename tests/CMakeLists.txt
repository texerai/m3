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
)

# Add test executables
add_executable(core_test core_test.cpp)
add_executable(data_test data_test.cpp)

add_executable(bridge_boom_test
    bridge_boom_test.cpp
    test_trace_reader/test_trace_reader.cpp
    ${CMAKE_SOURCE_DIR}/src/bridges/bridge_boom/bridge_boom.cpp
)

# Link with the main library
target_link_libraries(core_test PRIVATE m3)
target_link_libraries(data_test PRIVATE m3)
target_link_libraries(bridge_boom_test PRIVATE m3)

# Enable tests
enable_testing()
add_test(NAME core_test COMMAND core_test)
add_test(NAME data_test COMMAND data_test)
add_test(NAME bridge_boom_test COMMAND bridge_boom_test)
