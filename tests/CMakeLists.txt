cmake_minimum_required(VERSION 3.10)
project(M3Tests LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/bridges/bridge_boom
)

# Unit tests (doctest). The checks are real assertions that survive
# Release builds, unlike the C assert() macro.
add_executable(core_test core_test.cpp)
add_executable(data_test data_test.cpp)

# Trace replay test for the BOOM bridge.
add_executable(bridge_boom_test
    bridge_boom_test.cpp
    test_trace_reader/test_trace_reader.cpp
    ${CMAKE_SOURCE_DIR}/src/bridges/bridge_boom/bridge_boom.cpp
)

# Link with the main library
target_link_libraries(core_test PRIVATE m3)
target_link_libraries(data_test PRIVATE m3)
target_link_libraries(bridge_boom_test PRIVATE m3)

# Register tests
add_test(NAME core_test COMMAND core_test)
add_test(NAME data_test COMMAND data_test)

# One ctest entry per recorded trace. Each replay must finish with zero
# detected errors, otherwise the test fails.
file(GLOB M3_TRACES ${CMAKE_CURRENT_SOURCE_DIR}/traces/*.txt)
foreach(trace ${M3_TRACES})
    get_filename_component(trace_name ${trace} NAME_WE)
    add_test(NAME trace_${trace_name} COMMAND bridge_boom_test ${trace})
endforeach()

# towers.txt was recorded with a hook that samples the LDQ/STQ address on
# the first rising edge of addr_valid, which for a TLB miss is the virtual
# address; the physical address written later never produces a new edge.
# The replay therefore sees memops at addresses such as 0x0 and 0x8 and
# cannot pass until the trace is re-recorded. Keep it as an expected
# failure so a (surprising) pass is noticed.
set_tests_properties(trace_towers PROPERTIES WILL_FAIL TRUE)
