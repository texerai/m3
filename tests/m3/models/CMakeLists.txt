# Copyright (c) 2022 MicroArchitecture Santa Cruz & Kabylkas Labs
cmake_minimum_required(VERSION 3.0)

# List of projects.
project(marionette)

# Setting compiler flags.
set (CMAKE_CXX_COMPILER g++)
#set(CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${FLAGS})

# Setting project directories.
# Branch Predictor Marionettes.
add_subdirectory(./branch_predictor)
