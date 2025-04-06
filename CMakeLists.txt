cmake_minimum_required(VERSION 3.10)
project(Source)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(M3_SOURCES
    source/memory_marionette.cpp
    source/data.cpp
    source/notifier.cpp
    source/lrand.cpp
    source/rvutils.cpp
    source/tracer.cpp
)

# Create the Source library
add_library(m3 STATIC ${M3_SOURCES})

# Add include directories
target_include_directories(m3 PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

# Add subdirectories (including tests)
add_subdirectory(tests)