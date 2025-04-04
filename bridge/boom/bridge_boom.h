/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef BRIDGE_BOOM_T_H_
#define BRIDGE_BOOM_T_H_

// C++ libraries.
#include <memory>
#include <stdint.h>
#include <string>

// Forward declarations.
class dromajo_t;

namespace m3
{
    // Forward declarations.
    struct RtlHookData;

    class BridgeBoom
    {
    public:
        void Init(uint32_t ncores);
        void RegisterEvent(const RtlHookData& data);
        bool ServeRegisteredEvents();
        void Close();

    private:
        struct BridgeBoomImpl;
        BridgeBoomImpl* pimpl_ = nullptr;
    };

}
#endif // BRIDGE_BOOM_T_H_