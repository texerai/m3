/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef COMPLETE_STORE_H_
#define COMPLETE_STORE_H_

#include "m3command.h"

namespace m3
{
    // Forward declarations.
    struct RtlHookData;

    class CompleteStore : public IM3Command
    {
    public:
        CompleteStore() = delete;
        CompleteStore(const RtlHookData& data);
        bool Execute(State& state) override;
        bool Execute(State& state, Tracer& m3tracer) override;
    };
}

#endif