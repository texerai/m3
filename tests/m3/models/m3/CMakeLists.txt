# Copyright (c) 2023 MicroArchitecture Santa Cruz and Texer.ai.
cmake_minimum_required(VERSION 3.0)

# Project.
project(m3)

# Compiler settings.
set (CMAKE_CXX_COMPILER g++)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Setting project directories.
set (SRC_DIR ./source)
set (EXT_DIR ./external)
set (BRIDGE_DIR  ./bridge)
set (DROMAJO_DIR ../dromajo)

# Redirect to single build output location
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# Added includes, maybe are not needed
include_directories(${BRIDGE_DIR}/boom_m3_commands/)
include_directories(${BRIDGE_DIR}/)

# Set Version Header
# set(CONFIG_VERSION "Dromajo-0.1")
# configure_file(${DROMAJO_DIR}/include/config.h.in config.h @ONLY)

# Local target: m3.
# Setting all header files for this project.
set (ALL_H
    # Bridge files.
    # ${BRIDGE_DIR}/boom_m3_t.h
    # ${BRIDGE_DIR}/rtl_event.h
    # ${BRIDGE_DIR}/memop_info.h
    ${BRIDGE_DIR}/state.h
    ${BRIDGE_DIR}/core_m3_t.h

    # # Bridge commands.
    ${BRIDGE_DIR}/boom_m3_commands/m3command.h
    ${BRIDGE_DIR}/boom_m3_commands/add_address.h
    ${BRIDGE_DIR}/boom_m3_commands/add_store_data.h
    ${BRIDGE_DIR}/boom_m3_commands/commit_memop.h
    ${BRIDGE_DIR}/boom_m3_commands/complete_store.h
    ${BRIDGE_DIR}/boom_m3_commands/create_memop.h
    ${BRIDGE_DIR}/boom_m3_commands/perform_load.h
    ${BRIDGE_DIR}/boom_m3_commands/update_cacheline_state.h
    ${BRIDGE_DIR}/boom_m3_commands/update_cacheline_data.h
    ${BRIDGE_DIR}/boom_m3_commands/send_dcache_amo.h
    ${BRIDGE_DIR}/boom_m3_commands/complete_amo.h

    # Memory model.
    ${SRC_DIR}/Gold_core.hpp
    ${SRC_DIR}/explicit_type.hpp
    ${SRC_DIR}/Gold_data.hpp
    ${SRC_DIR}/Gold_mem.hpp
    ${SRC_DIR}/Gold_notify.hpp
    ${SRC_DIR}/lrand.hpp
    ${SRC_DIR}/robin_hood.hpp
    ${SRC_DIR}/rvutils.h

    ${SRC_DIR}/rvamo_utils.h
    ${SRC_DIR}/debug_utils.hpp
    # ${SRC_DIR}/tracer.h
)

# Setting all source files for this project.
set (ALL_CPP
    # Bridge files.
    # ${BRIDGE_DIR}/boom_m3_t.cpp
     ${BRIDGE_DIR}/core_m3_t.cpp

    # # Bridge commands.
    ${BRIDGE_DIR}/boom_m3_commands/add_address.cpp
    ${BRIDGE_DIR}/boom_m3_commands/add_store_data.cpp
    ${BRIDGE_DIR}/boom_m3_commands/commit_memop.cpp
    ${BRIDGE_DIR}/boom_m3_commands/complete_store.cpp
    ${BRIDGE_DIR}/boom_m3_commands/create_memop.cpp
    ${BRIDGE_DIR}/boom_m3_commands/perform_load.cpp
    ${BRIDGE_DIR}/boom_m3_commands/update_cacheline_state.cpp
    ${BRIDGE_DIR}/boom_m3_commands/update_cacheline_data.cpp
    ${BRIDGE_DIR}/boom_m3_commands/send_dcache_amo.cpp
    ${BRIDGE_DIR}/boom_m3_commands/complete_amo.cpp
    ${BRIDGE_DIR}/boom_m3_commands/recovery_rob.cpp

    # Memory model.
    ${SRC_DIR}/Gold_core.cpp
    ${SRC_DIR}/Gold_data.cpp
    ${SRC_DIR}/Gold_notify.cpp
    ${SRC_DIR}/lrand.cpp
    ${SRC_DIR}/rvutils.cpp

    ${SRC_DIR}/rvamo_utils.cpp
    ${SRC_DIR}/debug_utils.cpp
    # ${SRC_DIR}/tracer.cpp

    # External.
    ${EXT_DIR}/fmt/format.cc
    ${EXT_DIR}/fmt/os.cc
)

# Adding target.
# Static Library
add_library(m3 STATIC ${ALL_H} ${ALL_CPP} )# ${DROMAJO_ALL_H} ${DROMAJO_ALL_CPP})
#add_library(m3 SHARED ${ALL_H} ${ALL_CPP} )# ${DROMAJO_ALL_H} ${DROMAJO_ALL_CPP})
target_include_directories(m3 PRIVATE
    ${SRC_DIR}
    ${EXT_DIR}
    # ${BRIDGE_DIR}
    # ${BRIDGE_DIR}/boom_m3_commands
    # ${DROMAJO_DIR}/include
)
