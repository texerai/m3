cmake_minimum_required(VERSION 3.10)
project(GoldTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add test executables
add_executable(core_test core_test.cpp)
add_executable(data_test data_test.cpp)

# Link with the main project library
target_link_libraries(core_test PRIVATE Gold)
target_link_libraries(data_test PRIVATE Gold)

# Add include directories
target_include_directories(core_test PRIVATE ${CMAKE_SOURCE_DIR}/gold)
target_include_directories(data_test PRIVATE ${CMAKE_SOURCE_DIR}/gold)

# Enable testing
enable_testing()
add_test(NAME core_test COMMAND core_test)
add_test(NAME data_test COMMAND data_test) 