cmake_minimum_required(VERSION 3.10)
project(SourceTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add test executables
add_executable(core_test core_test.cpp)
add_executable(data_test data_test.cpp)
add_executable(bridge_boom_test
    bridge_boom_test.cpp
    ${CMAKE_SOURCE_DIR}/bridge/bridge_boom/bridge_boom.cpp
)

# Link with the main project library
target_link_libraries(core_test PRIVATE m3)
target_link_libraries(data_test PRIVATE m3)
target_link_libraries(bridge_boom_test PRIVATE m3)

# Add include directories
target_include_directories(core_test PRIVATE ${CMAKE_SOURCE_DIR}/source)
target_include_directories(data_test PRIVATE ${CMAKE_SOURCE_DIR}/source)
target_include_directories(bridge_boom_test PRIVATE
    ${CMAKE_SOURCE_DIR}/source
    ${CMAKE_SOURCE_DIR}/bridge/bridge_boom
)

# Enable testing
enable_testing()
add_test(NAME core_test COMMAND core_test)
add_test(NAME data_test COMMAND data_test) 
add_test(NAME bridge_boom_test COMMAND bridge_boom_test) 