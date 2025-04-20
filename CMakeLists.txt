cmake_minimum_required(VERSION 3.10)
project(M3 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build mode
set(BUILD_MODE Debug)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Gather all source files
set(M3_SOURCES
    src/m3/data.cpp
    src/m3/memory_marionette.cpp
    src/third_party/lrand.cpp
    src/utils/rvutils.cpp
    src/utils/tracer.cpp
)

# Define the static library
add_library(m3 STATIC ${M3_SOURCES})

# Optionally, link third-party headers (already covered via include)
target_include_directories(m3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add tests or other subdirectories
add_subdirectory(tests)
