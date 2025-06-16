/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef SEND_DCACHE_AMO_H_
#define SEND_DCACHE_AMO_H_

#include "m3command.h"

namespace m3
{
    // Forward declarations.
    struct RTLEventData;

    class SendDcacheAmo : public IM3Command
    {
    public:
        SendDcacheAmo() = delete;
        SendDcacheAmo(const RTLEventData& data);
        bool Execute(State& state) override;
    };
}

#endif