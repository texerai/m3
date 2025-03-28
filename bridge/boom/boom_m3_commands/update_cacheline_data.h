/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef UPDATE_CACHELINE_DATA_H_
#define UPDATE_CACHELINE_DATA_H_

#include "m3command.h"

namespace m3
{
    // Forward declarations.
    struct RTLEventData;

    class UpdateCachelineData : public IM3Command
    {
    public:
        UpdateCachelineData() = delete;
        UpdateCachelineData(const RTLEventData& data);
        bool Execute(State& state) override;
        bool Execute(State& state, Tracer& m3tracer) override;

    private:
        bool was_globally_performed_ = false;
        uint64_t performed_id_ = 0;
    };
}

#endif