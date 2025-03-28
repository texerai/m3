cmake_minimum_required(VERSION 3.10)
project(Gold)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(GOLD_SOURCES
    gold/Gold_core.cpp
    gold/Gold_data.cpp
    gold/Gold_notify.cpp
    gold/lrand.cpp
    gold/rvutils.cpp
    gold/tracer.cpp
)

# Create the Gold library
add_library(Gold STATIC ${GOLD_SOURCES})

# Add include directories
target_include_directories(Gold PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add subdirectories (including tests)
add_subdirectory(tests)