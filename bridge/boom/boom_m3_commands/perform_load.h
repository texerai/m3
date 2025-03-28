/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef PERFORM_LOAD_H_
#define PERFORM_LOAD_H_

#include "m3command.h"

namespace m3
{
    // Forward declarations.
    struct RTLEventData;

    class PerformLoad : public IM3Command
    {
    public:
        PerformLoad() = delete;
        PerformLoad(const RTLEventData& data);
        bool Execute(State& state) override;
        bool Execute(State& state, Tracer& m3tracer) override;
    };
}

#endif