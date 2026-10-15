cmake_minimum_required(VERSION 3.10)
project(M3 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to a Debug build so that the internal asserts stay enabled.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(M3_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(M3_BUILD_TESTS "Build the test executables" ON)

add_compile_options(-Wall -Wextra)
if(M3_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

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

target_include_directories(m3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add tests or other subdirectories
if(M3_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
