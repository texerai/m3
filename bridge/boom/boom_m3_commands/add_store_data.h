/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef ADD_STORE_DATA_H_
#define ADD_STORE_DATA_H_

#include "m3command.h"

namespace m3
{
    // Forward declarations.
    struct RTLEventData;

    class AddStoreData : public IM3Command
    {
    public:
        AddStoreData() = delete;
        AddStoreData(const RTLEventData& data);
        bool Execute(State& state) override;
        bool Execute(State& state, Tracer& m3tracer) override;
    };
}

#endif