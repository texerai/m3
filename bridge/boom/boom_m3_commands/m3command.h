/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#ifndef M3_COMMAND_H_
#define M3_COMMAND_H_

// Local libraries.
#include "rtl_event.h"

namespace m3
{
    // Forward declarations.
    struct State;
    class Tracer;

    class IM3Command
    {
    public:
        virtual bool Execute(State& state) = 0;
        virtual bool Execute(State& state, Tracer& m3tracer) = 0;
        RTLEventData GetData() const { return data_; }
        uint32_t GetPriority() const { return priority_; }

    protected:
        RTLEventData data_;
        uint32_t priority_ = 0;
    };
}
#endif