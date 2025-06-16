/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef ADD_ADDRESS_H_
#define ADD_ADDRESS_H_

#include "m3command.h"

namespace m3
{
    // Forward declarations.
    struct RTLEventData;

    class AddAddress : public IM3Command
    {
    public:
        AddAddress() = delete;
        AddAddress(const RTLEventData& data);
        bool Execute(State& state) override;
    };
}

#endif