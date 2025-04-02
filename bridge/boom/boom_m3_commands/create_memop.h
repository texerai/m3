/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef CREATE_MEMOP_H_
#define CREATE_MEMOP_H_

#include "m3command.h"

namespace m3
{
    // Forward declarations.
    struct RtlHookData;

    class CreateMemop : public IM3Command
    {
    public:
        CreateMemop() = delete;
        CreateMemop(const RtlHookData& data);
        bool Execute(State& state) override;
        bool Execute(State& state, Tracer& m3tracer) override;
    };
}

#endif