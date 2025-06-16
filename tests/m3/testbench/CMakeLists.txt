# Copyright (c) 2023 Kabylkas Labs. All rights reserved.
cmake_minimum_required(VERSION 3.0)

# List of projects.
project(marisim)

# Setting compiler flags.
set (CMAKE_CXX_COMPILER g++)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${FLAGS})

set (SRC_DIR .)

set (ALL_H
    # Local library headers.
    ${SRC_DIR}/simulator.h
    ${SRC_DIR}/fast_simulator.h
    ${SRC_DIR}/VTestHarness.h
)

set (ALL_CPP
    # Local library headers.
    ${SRC_DIR}/simulator.cpp
    ${SRC_DIR}/fast_simulator.cpp
)

add_executable(marisim sim_cli.cpp ${ALL_H} ${ALL_CPP})
