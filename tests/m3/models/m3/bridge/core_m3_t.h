/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef CORE_M3_T_H_
#define CORE_M3_T_H_

// C++ libraries.
#include <memory>
#include <stdint.h>
#include <string>
#include "state.h"

#include "debug_utils.hpp"

namespace m3
{
    // Forward declarations.
    struct RTLEventData;

    class core_m3_t
    {
    public:
        void init(uint32_t ncores, debug::VerbosityLevel level, debug::ExecutionMode mode);
        void register_event(const RTLEventData& data);
        bool serve_registered_events();
        void close();

        // required variables for spike run method when using spike with m3 hooks
        uint64_t core_id;
        // Used if the memory access is performed by an operation
        uint8_t  rob_id;

    private:
        struct core_m3_impl;
        core_m3_impl* pimpl_ = nullptr;
    };

    extern State* state;
    extern std::shared_ptr<m3::core_m3_t> m3_ptr;

}
#endif // CORE_M3_T_H_